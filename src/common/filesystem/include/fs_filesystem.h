/*
** fs_filesystem.h
**
** File system  I/O functions.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
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
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "fs_files.h"
#include "resourcefile.h"

namespace FileSys {
	
union LumpShortName
{
	char		String[9];

	uint32_t		dword;			// These are for accessing the first 4 or 8 chars of
	uint64_t		qword;			// Name as a unit without breaking strict aliasing rules
};


struct FolderEntry
{
	const char *name;
	unsigned lumpnum;
};

class FileSystem
{
public:
	FileSystem();
	~FileSystem ();

	// The wadnum for the IWAD
	int GetIwadNum() { return IwadIndex; }
	void SetIwadNum(int x) { IwadIndex = x; }

	int GetMaxIwadNum() { return MaxIwadIndex; }
	void SetMaxIwadNum(int x) { MaxIwadIndex = x; }

	bool InitSingleFile(const char *filename, FileSystemMessageFunc Printf = nullptr);
	bool InitMultipleFiles (std::vector<std::string>& filenames, LumpFilterInfo* filter = nullptr, FileSystemMessageFunc Printf = nullptr, bool allowduplicates = false);
	void AddFile (const char *filename, FileReader *wadinfo, LumpFilterInfo* filter, FileSystemMessageFunc Printf);
	int CheckIfResourceFileLoaded (const char *name) noexcept;
	void AddAdditionalFile(const char* filename, FileReader* wadinfo = NULL) {}

	const char *GetResourceFileName (int filenum) const noexcept;
	const char *GetResourceFileFullName (int wadnum) const noexcept;

	int GetFirstEntry(int wadnum) const noexcept;
	int GetLastEntry(int wadnum) const noexcept;
    int GetEntryCount(int wadnum) const noexcept;

	int CheckNumForName (const char *name, int namespc) const;
	int CheckNumForName (const char *name, int namespc, int wadfile, bool exact = true) const;
	int GetNumForName (const char *name, int namespc) const;

	inline int CheckNumForName (const uint8_t *name) const { return CheckNumForName ((const char *)name, ns_global); }
	inline int CheckNumForName (const char *name) const { return CheckNumForName (name, ns_global); }
	inline int CheckNumForName (const uint8_t *name, int ns) const { return CheckNumForName ((const char *)name, ns); }
	inline int GetNumForName (const char *name) const { return GetNumForName (name, ns_global); }
	inline int GetNumForName (const uint8_t *name) const { return GetNumForName ((const char *)name); }
	inline int GetNumForName (const uint8_t *name, int ns) const { return GetNumForName ((const char *)name, ns); }

	int CheckNumForFullName (const char *cname, bool trynormal = false, int namespc = ns_global, bool ignoreext = false) const;
	int CheckNumForFullName (const char *name, int wadfile) const;
	int GetNumForFullName (const char *name) const;
	int FindFile(const char* name) const
	{
		return CheckNumForFullName(name);
	}

	bool FileExists(const char* name) const
	{
		return FindFile(name) >= 0;
	}

	bool FileExists(const std::string& name) const
	{
		return FindFile(name.c_str()) >= 0;
	}

	LumpShortName& GetShortName(int i);	// may only be called before the hash chains are set up.
	void RenameFile(int num, const char* fn);
	bool CreatePathlessCopy(const char* name, int id, int flags);

	void ReadFile (int lump, void *dest);
	// These should only be used if the file data really needs padding.
	FileData ReadFile (int lump);
	FileData ReadFile (const char *name) { return ReadFile (GetNumForName (name)); }
	FileData ReadFileFullName(const char* name) { return ReadFile(GetNumForFullName(name)); }

	FileReader OpenFileReader(int lump, int readertype, int readerflags);		// opens a reader that redirects to the containing file's one.
	FileReader OpenFileReader(const char* name);
	FileReader ReopenFileReader(const char* name, bool alwayscache = false);
	FileReader OpenFileReader(int lump)
	{
		return OpenFileReader(lump, READER_SHARED, READERFLAG_SEEKABLE);
	}

	FileReader ReopenFileReader(int lump, bool alwayscache = false)
	{
		return OpenFileReader(lump, alwayscache ? READER_CACHED : READER_NEW, READERFLAG_SEEKABLE);
	}


	int FindLump (const char *name, int *lastlump, bool anyns=false);		// [RH] Find lumps with duplication
	int FindLumpMulti (const char **names, int *lastlump, bool anyns = false, int *nameindex = NULL); // same with multiple possible names
	int FindLumpFullName(const char* name, int* lastlump, bool noext = false);
	bool CheckFileName (int lump, const char *name);	// [RH] True if lump's name == name

	int FindFileWithExtensions(const char* name, const char* const* exts, int count) const;
	int FindResource(int resid, const char* type, int filenum = -1) const noexcept;
	int GetResource(int resid, const char* type, int filenum = -1) const;


	static uint32_t LumpNameHash (const char *name);		// [RH] Create hash key from an 8-char name

	ptrdiff_t FileLength (int lump) const;
  uint32_t FileHash (int lump) const;
	int GetFileFlags (int lump);					// Return the flags for this lump
	const char* GetFileShortName(int lump) const;
	const char *GetFileFullName (int lump, bool returnshort = true) const;	// [RH] Returns the lump's full name
	std::string GetFileFullPath (int lump) const;		// [RH] Returns wad's name + lump's full name
	int GetFileContainer (int lump) const;				// [RH] Returns wadnum for a specified lump
	int GetFileNamespace (int lump) const;			// [RH] Returns the namespace a lump belongs to
	void SetFileNamespace(int lump, int ns);
	int GetResourceId(int lump) const;				// Returns the RFF index number for this lump
	const char* GetResourceType(int lump) const;
	bool CheckFileName (int lump, const char *name) const;	// [RH] Returns true if the names match
	unsigned GetFilesInFolder(const char *path, std::vector<FolderEntry> &result, bool atomic) const;

	int GetNumEntries() const
	{
		return NumEntries;
	}

	int GetNumWads() const
	{
		return (int)Files.size();
	}

	int AddFromBuffer(const char* name, char* data, int size, int id, int flags);
	FileReader* GetFileReader(int wadnum);	// Gets a FileReader object to the entire WAD
	void InitHashChains();

protected:

	struct LumpRecord;

	std::vector<FResourceFile *> Files;
	std::vector<LumpRecord> FileInfo;

	std::vector<uint32_t> Hashes;	// one allocation for all hash lists.
	uint32_t *FirstLumpIndex = nullptr;	// [RH] Hashing stuff moved out of lumpinfo structure
	uint32_t *NextLumpIndex = nullptr;

	uint32_t *FirstLumpIndex_FullName = nullptr;	// The same information for fully qualified paths from .zips
	uint32_t *NextLumpIndex_FullName = nullptr;

	uint32_t *FirstLumpIndex_NoExt = nullptr;	// The same information for fully qualified paths from .zips
	uint32_t *NextLumpIndex_NoExt = nullptr;

	uint32_t* FirstLumpIndex_ResId = nullptr;	// The same information for fully qualified paths from .zips
	uint32_t* NextLumpIndex_ResId = nullptr;

	uint32_t NumEntries = 0;					// Not necessarily the same as FileInfo.Size()
	uint32_t NumWads = 0;

	int IwadIndex = -1;
	int MaxIwadIndex = -1;

	StringPool* stringpool = nullptr;

private:
	void DeleteAll();
	void MoveLumpsInFolder(const char *);

};

}
