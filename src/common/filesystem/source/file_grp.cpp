/*
** file_grp.cpp
**
**
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

#include "resourcefile.h"
#include "fs_swap.h"

namespace FileSys {
	using namespace byteswap;

//==========================================================================
//
//
//
//==========================================================================

struct GrpHeader
{
	uint32_t		Magic[3];
	uint32_t		NumLumps;
};

struct GrpLump
{
	union
	{
		struct
		{
			char		Name[12];
			uint32_t		Size;
		};
		char NameWithZero[13];
	};
};


//==========================================================================
//
// Open it
//
//==========================================================================

static bool OpenGrp(FResourceFile* file, LumpFilterInfo* filter)
{
	GrpHeader header;

	auto Reader = file->GetContainerReader();
	Reader->Read(&header, sizeof(header));
	uint32_t NumLumps = LittleLong(header.NumLumps);
	auto Entries = file->AllocateEntries(NumLumps);

	GrpLump *fileinfo = new GrpLump[NumLumps];
	Reader->Read (fileinfo, NumLumps * sizeof(GrpLump));

	int Position = sizeof(GrpHeader) + NumLumps * sizeof(GrpLump);

	for(uint32_t i = 0; i < NumLumps; i++)
	{
		Entries[i].Position = Position;
		Entries[i].CompressedSize = Entries[i].Length = LittleLong(fileinfo[i].Size);
		Position += fileinfo[i].Size;
		Entries[i].Flags = 0;
		Entries[i].Namespace = ns_global;
		fileinfo[i].NameWithZero[12] = '\0';	// Be sure filename is null-terminated
		Entries[i].ResourceID = -1;
		Entries[i].Method = METHOD_STORED;
		Entries[i].FileName = file->NormalizeFileName(fileinfo[i].Name);
	}
	file->GenerateHash();
	delete[] fileinfo;
	return true;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckGRP(const char *filename, FileReader &file, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
	char head[12];

	if (file.GetLength() >= 12)
	{
		file.Seek(0, FileReader::SeekSet);
		file.Read(&head, 12);
		file.Seek(0, FileReader::SeekSet);
		if (!memcmp(head, "KenSilverman", 12))
		{
			auto rf = new FResourceFile(filename, file, sp);
			if (OpenGrp(rf, filter)) return rf;
			file = rf->Destroy();
		}
	}
	return nullptr;
}

}
