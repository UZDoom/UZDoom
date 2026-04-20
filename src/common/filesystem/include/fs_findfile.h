/*
** fs_findfile.h
**
** Directory searching routines
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace FileSys {
	
struct FileListEntry
{
	std::string FileName;		// file name only
	std::string FilePath;		// full path to file
	std::string FilePathRel;	// path relative to the scanned directory.
	size_t Length = 0;
	bool isDirectory = false;
	bool isReadonly = false;
	bool isHidden = false;
	bool isSystem = false;
};

using FileList = std::vector<FileListEntry>;

struct FCompressedBuffer;
bool ScanDirectory(std::vector<FileListEntry>& list, const char* dirpath, const char* match, bool nosubdir = false, bool readhidden = false);
bool FS_DirEntryExists(const char* pathname, bool* isdir);

inline void FixPathSeparator(char* path)
{
	while (*path)
	{
		if (*path == '\\')
			*path = '/';
		path++;
	}
}

}
