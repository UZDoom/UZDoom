/*
** file_directory.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2008-2016 Marisa Heit
** Copyright 2009-2016 Christoph Oelckers
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


#include <sys/stat.h>

#include "resourcefile.h"
#include "fs_findfile.h"
#include "fs_stringpool.h"

namespace FileSys {
	
std::string FS_FullPath(const char* directory);

//==========================================================================
//
// Zip file
//
//==========================================================================

class FDirectory : public FResourceFile
{
	const bool nosubdir;
	const char* mBasePath;
	const char** SystemFilePath;


	int AddDirectory(const char* dirpath, LumpFilterInfo* filter, FileSystemMessageFunc Printf);

public:
	FDirectory(const char * dirname, StringPool* sp, bool nosubdirflag = false);
	bool Open(LumpFilterInfo* filter, FileSystemMessageFunc Printf);
	FileReader GetEntryReader(uint32_t entry, int, int) override;
};



//==========================================================================
//
// 
//
//==========================================================================

FDirectory::FDirectory(const char * directory, StringPool* sp, bool nosubdirflag)
	: FResourceFile("", sp), nosubdir(nosubdirflag)
{
	auto fn = FS_FullPath(directory);
	if (fn.back() != '/') fn += '/';
	FileName = stringpool->Strdup(fn.c_str());
}

//==========================================================================
//
// Windows version
//
//==========================================================================

int FDirectory::AddDirectory(const char *dirpath, LumpFilterInfo* filter, FileSystemMessageFunc Printf)
{
	int count = 0;

	FileList list;
	if (!ScanDirectory(list, dirpath, "*"))
	{
		Printf(FSMessageLevel::Error, "Could not scan '%s': %s\n", dirpath, strerror(errno));
	}
	else
	{
		mBasePath = nullptr;
		AllocateEntries((int)list.size());
		SystemFilePath = (const char**)stringpool->Alloc(list.size() * sizeof(const char*));
		for(auto& entry : list)
		{
			if (mBasePath == nullptr)
			{
				// extract the base path from the first entry to cover changes made in ScanDirectory.
				auto full = entry.FilePath.rfind(entry.FilePathRel);
				std::string path(entry.FilePath, 0, full);
				mBasePath = stringpool->Strdup(path.c_str());
			}
			if (!entry.isDirectory)
			{
				auto fi = entry.FileName;
				for (auto& c : fi) c = tolower(c);
				if (strstr(fi.c_str(), ".orig") || strstr(fi.c_str(), ".bak") || strstr(fi.c_str(), ".cache"))
				{
					// We shouldn't add backup files to the file system
					continue;
				}


				if (filter == nullptr || filter->filenamecheck == nullptr || filter->filenamecheck(fi.c_str(), entry.FilePath.c_str()))
				{
					if (entry.Length > 0x7fffffff)
					{
						Printf(FSMessageLevel::Warning, "%s is larger than 2GB and will be ignored\n", entry.FilePath.c_str());
						continue;
					}
					// for accessing the file we need to retain the original unaltered path.
					// On Linux this is important because its file system is case sensitive,
					// but even on Windows the Unicode normalization is destructive 
					// for some characters and cannot be used for file names.
					// Examples for this are the Turkish 'i's or the German ß.
					SystemFilePath[count] = stringpool->Strdup(entry.FilePathRel.c_str());
					// for internal access we use the normalized form of the relative path.
					// this is fine because the paths that get compared against this will also be normalized.
					Entries[count].FileName = NormalizeFileName(entry.FilePathRel.c_str());
					Entries[count].CompressedSize = Entries[count].Length = entry.Length;
					Entries[count].Flags = RESFF_FULLPATH;
					Entries[count].ResourceID = -1;
					Entries[count].Method = METHOD_STORED;
					Entries[count].Namespace = ns_global;
					Entries[count].Position = count;
					count++;
				}
			}
		}
	}
	return count;
}

//==========================================================================
//
//
//
//==========================================================================

bool FDirectory::Open(LumpFilterInfo* filter, FileSystemMessageFunc Printf)
{
	NumLumps = AddDirectory(FileName, filter, Printf);
	GenerateHash(true);
	PostProcessArchive(filter);
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

FileReader FDirectory::GetEntryReader(uint32_t entry, int readertype, int)
{
	FileReader fr;
	if (entry < NumLumps)
	{
		std::string fn = mBasePath;
		fn += SystemFilePath[Entries[entry].Position];
		fr.OpenFile(fn.c_str());
		if (readertype == READER_CACHED)
		{
			auto data = fr.Read();
			fr.OpenMemoryArray(data);
		}
	}
	return fr;
}

//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckDir(const char *filename, bool nosubdirflag, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
	auto rf = new FDirectory(filename, sp, nosubdirflag);
	if (rf->Open(filter, Printf)) return rf;
	delete rf;
	return nullptr;
}

}
