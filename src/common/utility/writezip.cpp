/*
** writezip.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2023 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include <stdint.h>
#include <algorithm>
#include "tarray.h"
#include "files.h"
#include "m_swap.h"
#include "w_zip.h"
#include "fs_decompress.h"
#include "cmdlib.h"

using FileSys::FCompressedBuffer;



//==========================================================================
//
// time_to_dos
//
// Converts time from struct tm to the DOS format used by zip files.
//
//==========================================================================

static std::pair<uint16_t, uint16_t> time_to_dos(struct tm *time)
{
	std::pair<uint16_t, uint16_t> val;
	if (time == NULL || time->tm_year < 80)
	{
		val.first = val.second = 0;
	}
	else
	{
		val.first = time->tm_hour * 2048 + time->tm_min * 32 + time->tm_sec / 2;
		val.second = (time->tm_year - 80) * 512 + (time->tm_mon + 1) * 32 + time->tm_mday;
	}
	return val;
}

//==========================================================================
//
// append_to_zip
//
// Write a given file to the zipFile.
// 
// zipfile: zip object to be written to
// 
// returns: position = success, -1 = error
//
//==========================================================================

static int AppendToZip(FileWriter *zip_file, const FCompressedBuffer &content, std::pair<uint16_t, uint16_t> &dostime)
{
	FZipLocalFileHeader local;
	int position;

	int flags = 0;
	int method = content.mMethod;
	if (method >= FileSys::METHOD_IMPLODE_MIN && method <= FileSys::METHOD_IMPLODE_MAX)
	{
		flags = method - FileSys::METHOD_IMPLODE_MIN;
		method = FileSys::METHOD_IMPLODE;
	}
	else if (method == FileSys::METHOD_DEFLATE)
	{
		flags = 2;
	}
	else if (method >= 1337)
		return -1;

	local.Magic = ZIP_LOCALFILE;
	local.VersionToExtract[0] = 20;
	local.VersionToExtract[1] = 0;
	local.Flags = LittleShort((uint16_t)flags);
	local.Method = LittleShort((uint16_t)method);
	local.ModDate = LittleShort(dostime.first);
	local.ModTime = LittleShort(dostime.second);
	local.CRC32 = content.mCRC32;
	local.UncompressedSize = LittleLong((unsigned)content.mSize);
	local.CompressedSize = LittleLong((unsigned)content.mCompressedSize);
	local.NameLength = LittleShort((unsigned short)strlen(content.filename));
	local.ExtraLength = 0;

	// Fill in local directory header.

	position = (int)zip_file->Tell();

	// Write out the header, file name, and file data.
	if (zip_file->Write(&local, sizeof(local)) != sizeof(local) ||
		zip_file->Write(content.filename, strlen(content.filename)) != strlen(content.filename) ||
		zip_file->Write(content.mBuffer, content.mCompressedSize) != content.mCompressedSize)
	{
		return -1;
	}
	return position;
}


//==========================================================================
//
// write_central_dir
//
// Writes the central directory entry for a file.
//
//==========================================================================

int AppendCentralDirectory(FileWriter *zip_file, const FCompressedBuffer &content, std::pair<uint16_t, uint16_t> &dostime, int position)
{
	FZipCentralDirectoryInfo dir;

	int flags = 0;
	int method = content.mMethod;
	if (method >= FileSys::METHOD_IMPLODE_MIN && method <= FileSys::METHOD_IMPLODE_MAX)
	{
		flags = method - FileSys::METHOD_IMPLODE_MIN;
		method = FileSys::METHOD_IMPLODE;
	}
	else if (method == FileSys::METHOD_DEFLATE)
	{
		flags = 2;
	}
	else if (method >= 1337)
		return -1;

	dir.Magic = ZIP_CENTRALFILE;
	dir.VersionMadeBy[0] = 20;
	dir.VersionMadeBy[1] = 0;
	dir.VersionToExtract[0] = 20;
	dir.VersionToExtract[1] = 0;
	dir.Flags = LittleShort((uint16_t)flags);
	dir.Method = LittleShort((uint16_t)method);
	dir.ModTime = LittleShort(dostime.first);
	dir.ModDate = LittleShort(dostime.second);
	dir.CRC32 = content.mCRC32;
	dir.CompressedSize32 = LittleLong((unsigned)content.mCompressedSize);
	dir.UncompressedSize32 = LittleLong((unsigned)content.mSize);
	dir.NameLength = LittleShort((unsigned short)strlen(content.filename));
	dir.ExtraLength = 0;
	dir.CommentLength = 0;
	dir.StartingDiskNumber = 0;
	dir.InternalAttributes = 0;
	dir.ExternalAttributes = 0;
	dir.LocalHeaderOffset32 = LittleLong((unsigned)position);

	if (zip_file->Write(&dir, sizeof(dir)) != sizeof(dir) ||
		zip_file->Write(content.filename,  strlen(content.filename)) != strlen(content.filename))
	{
		return -1;
	}
	return 0;
}

bool WriteZip(const char* filename, const FCompressedBuffer* content, size_t contentcount)
{
	// try to determine local time
	struct tm *ltime;
	time_t ttime;
	ttime = time(nullptr);
	ltime = localtime(&ttime);
	auto dostime = time_to_dos(ltime);

	TArray<int> positions;

	auto f = FileWriter::Open(filename);
	if (f != nullptr)
	{
		for (size_t i = 0; i < contentcount; i++)
		{
			int pos = AppendToZip(f, content[i], dostime);
			if (pos == -1)
			{
				delete f;
				RemoveFile(filename);
				return false;
			}
			positions.Push(pos);
		}

		int dirofs = (int)f->Tell();
		for (size_t i = 0; i < contentcount; i++)
		{
			if (AppendCentralDirectory(f, content[i], dostime, positions[i]) < 0)
			{
				delete f;
				RemoveFile(filename);
				return false;
			}
		}

		// Write the directory terminator.
		FZipEndOfCentralDirectory dirend;
		dirend.Magic = ZIP_ENDOFDIR;
		dirend.DiskNumber = 0;
		dirend.FirstDisk = 0;
		dirend.NumEntriesOnAllDisks = dirend.NumEntries = LittleShort((uint16_t)contentcount);
		dirend.DirectoryOffset = LittleLong((unsigned)dirofs);
		dirend.DirectorySize = LittleLong((uint32_t)(f->Tell() - dirofs));
		dirend.ZipCommentLength = 0;
		if (f->Write(&dirend, sizeof(dirend)) != sizeof(dirend))
		{
			delete f;
			RemoveFile(filename);
			return false;
		}
		delete f;
		return true;
	}
	return false;
}
