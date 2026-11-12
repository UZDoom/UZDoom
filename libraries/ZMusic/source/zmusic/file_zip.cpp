/*
** file_zip.cpp
**
**---------------------------------------------------------------------------
** Copyright 1998-2009 Marisa Heit
** Copyright 2005-2023 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
**
*/

#include <time.h>
#include <miniz.h>
#include <vector>
#include <string>
#include "file_zip.h"
#include "w_zip.h"

namespace ZipFile {


#if defined __BIG_ENDIAN__

inline unsigned short LittleShort (unsigned short x)
{
	return (unsigned short)((x>>8) | (x<<8));
}

inline unsigned int LittleLong (unsigned int x)
{
	return (unsigned int)(
		(x>>24)
		| ((x>>8) & 0xff00)
		| ((x<<8) & 0xff0000)
		| (x<<24));
}

#else

inline unsigned short LittleShort(unsigned short x)
{
	return x;
}

inline unsigned int LittleLong(unsigned int x)
{
	return x;
}

#endif


#define BUFREADCOMMENT (0x400)

enum ELumpFlags
{
	RESFF_COMPRESSED = 1,	// compressed or encrypted, i.e. cannot be read with the container file's reader.
	RESFF_NEEDFILESTART = 2,	// The real position is not known yet and needs to be calculated on access
};

#ifdef _WIN32
#define fseek _fseeki64
#define ftell _ftelli64
#endif


static const int METHOD_STORED = 0;
static const int METHOD_DEFLATE = 8;

struct FResourceEntry
{
	size_t Length;
	size_t CompressedSize;
	std::string FileName;
	size_t Position;
	uint32_t CRC32;
	uint16_t Flags;
};


//==========================================================================
//
// Zip file
//
//==========================================================================

class FZipFile
{
	void SetEntryAddress(uint32_t entry);
	FILE* Reader;
	std::vector<FResourceEntry> Entries;

public:
	FZipFile(FILE* file);
	bool Open();
	size_t Read(uint32_t entry, void* data, size_t length);
	int FindEntry(const char* name);
	size_t Length(uint32_t entry)
	{
		return (entry < Entries.size()) ? Entries[entry].Length : 0;
	}
};

//-----------------------------------------------------------------------
//
// Finds the central directory end record in the end of the file.
// Taken from Quake3 source but the file in question is not GPL'ed. ;)
//
//-----------------------------------------------------------------------

static uint32_t Zip_FindCentralDir(FILE* fin, bool* zip64)
{
	unsigned char buf[BUFREADCOMMENT + 4];
	uint32_t FileSize;
	uint32_t uBackRead;
	uint32_t uMaxBack; // maximum size of global comment
	uint32_t uPosFound=0;

	auto pos = ftell(fin);
	fseek(fin, 0, SEEK_END);
	FileSize = ftell(fin);
	fseek(fin, pos, SEEK_SET);
	uMaxBack = std::min<uint32_t>(0xffff, FileSize);

	uBackRead = 4;
	while (uBackRead < uMaxBack)
	{
		uint32_t uReadSize, uReadPos;
		int i;
		if (uBackRead + BUFREADCOMMENT > uMaxBack) 
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		uReadPos = FileSize - uBackRead;

		uReadSize = std::min<uint32_t>((BUFREADCOMMENT + 4), (FileSize - uReadPos));

		if (fseek(fin, uReadPos, SEEK_SET) != 0) break;

		if (fread(buf, 1, (int32_t)uReadSize, fin) != (int32_t)uReadSize) break;

		for (i = (int)uReadSize - 3; (i--) > 0;)
		{
			if (buf[i] == 'P' && buf[i+1] == 'K' && buf[i+2] == 5 && buf[i+3] == 6 && !*zip64 && uPosFound == 0)
			{
				*zip64 = false;
				uPosFound = uReadPos + i;
			}
			if (buf[i] == 'P' && buf[i+1] == 'K' && buf[i+2] == 6 && buf[i+3] == 6)
			{
				*zip64 = true;
				uPosFound = uReadPos + i;
				return uPosFound;
			}
		}
	}
	return uPosFound;
}

//==========================================================================
//
// Zip file
//
//==========================================================================

FZipFile::FZipFile(FILE* file)
{
	Reader = std::move(file);
}

bool FZipFile::Open()
{
	bool zip64 = false;
	uint32_t centraldir = Zip_FindCentralDir(Reader, &zip64);
	int skipped = 0;

	if (centraldir == 0)
	{
		return false;
	}

	uint64_t dirsize, DirectoryOffset;
	uint32_t NumLumps;
	if (!zip64)
	{
		FZipEndOfCentralDirectory info;
		// Read the central directory info.
		fseek(Reader, centraldir, SEEK_SET);
		fread(&info, 1, sizeof(FZipEndOfCentralDirectory), Reader);

		// No multi-disk zips!
		if (info.NumEntries != info.NumEntriesOnAllDisks ||
			info.FirstDisk != 0 || info.DiskNumber != 0)
		{
			return false;
		}
		
		NumLumps = LittleShort(info.NumEntries);
		dirsize = LittleLong(info.DirectorySize);
		DirectoryOffset = LittleLong(info.DirectoryOffset);
	}
	else
	{
		FZipEndOfCentralDirectory64 info;
		// Read the central directory info.
		fseek(Reader, centraldir, SEEK_SET);
		fread(&info, 1, sizeof(FZipEndOfCentralDirectory64), Reader);

		// No multi-disk zips!
		if (info.NumEntries != info.NumEntriesOnAllDisks ||
			info.FirstDisk != 0 || info.DiskNumber != 0)
		{
			return false;
		}
		
		NumLumps = (uint32_t)info.NumEntries;
		dirsize = info.DirectorySize;
		DirectoryOffset = info.DirectoryOffset;
	}
	// Load the entire central directory. Too bad that this contains variable length entries...
	void *directory = malloc(dirsize);
	fseek(Reader, DirectoryOffset, SEEK_SET);
	fread(directory, 1, dirsize, Reader);

	char *dirptr = (char*)directory;

	Entries.resize(NumLumps);
	auto Entry = Entries.data();
	for (uint32_t i = 0; i < NumLumps; i++)
	{
		FZipCentralDirectoryInfo *zip_fh = (FZipCentralDirectoryInfo *)dirptr;

		int len = LittleShort(zip_fh->NameLength);
		std::string name(dirptr + sizeof(FZipCentralDirectoryInfo), len);
		dirptr += sizeof(FZipCentralDirectoryInfo) + 
				  LittleShort(zip_fh->NameLength) + 
				  LittleShort(zip_fh->ExtraLength) + 
				  LittleShort(zip_fh->CommentLength);

		if (dirptr > ((char*)directory) + dirsize)	// This directory entry goes beyond the end of the file.
		{
			free(directory);
			return false;
		}

		// skip Directories
		if (name.empty() || (name.back() == '/' && LittleLong(zip_fh->UncompressedSize32) == 0))
		{
			skipped++;
			continue;
		}

		// Ignore unknown compression formats
		zip_fh->Method = LittleShort(zip_fh->Method);
		if (zip_fh->Method != METHOD_STORED &&
			zip_fh->Method != METHOD_DEFLATE)
		{
			skipped++;
			continue;
		}
		// Also ignore encrypted entries
		zip_fh->Flags = LittleShort(zip_fh->Flags);
		if (zip_fh->Flags & ZF_ENCRYPTED)
		{
			skipped++;
			continue;
		}

		uint32_t UncompressedSize =LittleLong(zip_fh->UncompressedSize32);
		uint32_t CompressedSize = LittleLong(zip_fh->CompressedSize32);
		uint64_t LocalHeaderOffset = LittleLong(zip_fh->LocalHeaderOffset32);
		if (zip_fh->ExtraLength > 0)
		{
			uint8_t* rawext = (uint8_t*)zip_fh + sizeof(*zip_fh) + zip_fh->NameLength;
			uint32_t ExtraLength = LittleLong(zip_fh->ExtraLength);
			
			while (ExtraLength > 0)
			{
				auto zip_64 = (FZipCentralDirectoryInfo64BitExt*)rawext;
				uint32_t BlockLength = LittleLong(zip_64->Length);
				rawext += BlockLength + 4;
				ExtraLength -= BlockLength + 4;
				if (LittleLong(zip_64->Type) == 1 && BlockLength >= 0x18)
				{
					if (zip_64->CompressedSize > 0x7fffffff || zip_64->UncompressedSize > 0x7fffffff)
					{
						// The file system is limited to 32 bit file sizes;
						skipped++;
						continue;
					}
					UncompressedSize = (uint32_t)zip_64->UncompressedSize;
					CompressedSize = (uint32_t)zip_64->CompressedSize;
					LocalHeaderOffset = zip_64->LocalHeaderOffset;
				}
			}
		}

		Entry->FileName = name;
		Entry->Length = UncompressedSize;
		// The start of the Reader will be determined the first time it is accessed.
		Entry->Flags = RESFF_NEEDFILESTART;
		int Method = uint8_t(zip_fh->Method);
		if (Method != METHOD_STORED) Entry->Flags |= RESFF_COMPRESSED;
		Entry->CRC32 = zip_fh->CRC32;
		Entry->CompressedSize = CompressedSize;
		Entry->Position = LocalHeaderOffset;

		Entry++;

	}
	// Resize the lump record array to its actual size
	NumLumps -= skipped;
	free(directory);

	return true;
}

//==========================================================================
//
// SetLumpAddress
//
//==========================================================================

void FZipFile::SetEntryAddress(uint32_t entry)
{
	// This file is inside a zip and has not been opened before.
	// Position points to the start of the local file header, which we must
	// read and skip so that we can get to the actual file data.
	FZipLocalFileHeader localHeader;
	int skiplen;

	fseek(Reader, Entries[entry].Position, SEEK_SET);
	fread(&localHeader, 1, sizeof(localHeader), Reader);
	skiplen = LittleShort(localHeader.NameLength) + LittleShort(localHeader.ExtraLength);
	Entries[entry].Position += sizeof(localHeader) + skiplen;
	Entries[entry].Flags &= ~RESFF_NEEDFILESTART;
}


//==========================================================================
//
// DecompressorZ
//
// The zlib wrapper
// reads data from a ZLib compressed stream
//
//==========================================================================

static size_t DecompressZip(FILE* file, void* buffer, size_t len)
{
	const int BUFF_SIZE = 4096;
	bool SawEOF = false;
	z_stream Stream;
	uint8_t InBuff[BUFF_SIZE];

	if (len == 0) return 0;

	auto numread = fread (InBuff, 1, BUFF_SIZE, file);

	if (numread < BUFF_SIZE)
	{
		SawEOF = true;
	}
	Stream.next_in = InBuff;
	Stream.avail_in = (uInt)numread;

	Stream.zalloc = Z_NULL;
	Stream.zfree = Z_NULL;

	int err = inflateInit2 (&Stream, -MAX_WBITS);

	if (err < Z_OK)
	{
		return false;
	}

	auto olen = len;
	while (len > 0)
	{
		Stream.next_out = (Bytef*)buffer;
		unsigned rlen = (unsigned)std::min<ptrdiff_t>(len, 0x40000000);
		Stream.avail_out = rlen;
		buffer = (char*)Stream.next_out + rlen;
		len -= rlen;

		do
		{
			err = inflate(&Stream, Z_SYNC_FLUSH);
			if (Stream.avail_in == 0 && !SawEOF)
			{
				numread = fread (InBuff, 1, BUFF_SIZE, file);

				if (numread < BUFF_SIZE)
				{
					SawEOF = true;
				}
				Stream.next_in = InBuff;
				Stream.avail_in = (uInt)numread;
			}
		} while (err == Z_OK && Stream.avail_out != 0);
	}

	if (err != Z_OK && err != Z_STREAM_END || Stream.avail_out != 0)
	{
		inflateEnd (&Stream);
		return 0;
	}
	return olen;
}

size_t FZipFile::Read(uint32_t entry, void* data, size_t length)
{
	if (entry < Entries.size())
	{
		if (Entries[entry].Flags & RESFF_NEEDFILESTART)
		{
			SetEntryAddress(entry);
		}
		fseek(Reader, Entries[entry].Position, SEEK_SET);
		if (!(Entries[entry].Flags & RESFF_COMPRESSED))
		{
			return fread(data, 1, length, Reader);
		}
		else
		{
			return DecompressZip(Reader, data, length);
		}
	}
	return 0;
}


//==========================================================================
//
// 
//
//==========================================================================

int FZipFile::FindEntry(const char *name)
{
	for (unsigned i = 0; i < Entries.size(); i++)
	{
		if (!stricmp(name, Entries[i].FileName.c_str()))
		{
			return i;
		}
	}
	return -1;
}


//==========================================================================
//
// File open
//
//==========================================================================

FZipFile *Open(FILE* file)
{
	char head[4];
	FZipLocalFileHeader hdr;

	if (fread(&hdr, 1, sizeof(hdr), file) < sizeof(hdr))
	{
		fclose(file);
		return nullptr;
	}
	if (hdr.Magic != ZIP_LOCALFILE)
	{
		fclose(file);
		return nullptr;
	}
	auto rf = new FZipFile(file);
	if (rf->Open()) return rf;
	delete rf;
	fclose(file);
	return nullptr;
}

size_t Read(FZipFile* zip, uint32_t entry, void* data, size_t length)
{
	return zip ? zip->Read(entry, data, length) : 0;
}

int FindEntry(FZipFile* zip, const char* name)
{
	return zip ? zip->FindEntry(name) : -1;
}

size_t Length(FZipFile* zip, uint32_t entry)
{
	return zip ? zip->Length(entry) : 0;
}
void Close(FZipFile* zip)
{
	if (zip) delete zip;
}


}
