/*
** file_whres.cpp
**
** reads a Witchaven/TekWar sound resource file
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2019 Christoph Oelckers
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

#include "resourcefile.h"
#include "fs_stringpool.h"
#include "fs_swap.h"

namespace FileSys {
	using namespace byteswap;

//==========================================================================
//
// Open it
//
//==========================================================================

bool OpenWHRes(FResourceFile* file, LumpFilterInfo*)
{
	uint32_t directory[1024];

	auto BaseName = ExtractBaseName(file->GetFileName());
	auto Reader = file->GetContainerReader();
	Reader->Seek(-4096, FileReader::SeekEnd);
	Reader->Read(directory, 4096);

	int nl =1024/3;
	
	int k;
	for (k = 0; k < nl; k++)
	{
		uint32_t offset = LittleLong(directory[k * 3]) * 4096;
		uint32_t length = LittleLong(directory[k * 3 + 1]);
		if (length == 0)
		{
			break;
		}
	}
	auto Entries = file->AllocateEntries(k);
	auto NumLumps = k;

	int i = 0;
	for(k = 0; k < NumLumps; k++)
	{
		uint32_t offset = LittleLong(directory[k*3]) * 4096;
		uint32_t length = LittleLong(directory[k*3+1]);
		char num[6];
		snprintf(num, 6, "/%04d", k);
		std::string synthname = BaseName + num;
		
		Entries[i].Position = offset;
		Entries[i].CompressedSize = Entries[i].Length = length;
		Entries[i].Flags = RESFF_FULLPATH;
		Entries[i].Namespace = ns_global;
		Entries[i].ResourceID = -1;
		Entries[i].Method = METHOD_STORED;
		Entries[i].FileName = file->NormalizeFileName(synthname.c_str());
		i++;
	}
	return true;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckWHRes(const char *filename, FileReader &file, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
	if (file.GetLength() >= 8192) // needs to be at least 8192 to contain one file and the directory.
	{
		unsigned directory[1024];
		int nl =1024/3;

		file.Seek(-4096, FileReader::SeekEnd);
		file.Read(directory, 4096);
		auto size = file.GetLength();

		uint32_t checkpos = 0;
		for(int k = 0; k < nl; k++)
		{
			unsigned offset = LittleLong(directory[k*3]);
			unsigned length = LittleLong(directory[k*3+1]);
			if (length <= 0 && offset == 0) break;
			if (offset != checkpos || length == 0 || offset + length >= (size_t)size - 4096 ) return nullptr;
			checkpos += (length+4095) / 4096;
		}
		auto rf = new FResourceFile(filename, file, sp);
		if (OpenWHRes(rf, filter)) return rf;
		file = rf->Destroy();
	}
	return NULL;
}
 
}
