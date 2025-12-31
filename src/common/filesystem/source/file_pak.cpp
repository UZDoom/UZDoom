/*
** file_pak.cpp
**
**
**
**---------------------------------------------------------------------------
**
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

#include "resourcefile.h"

namespace FileSys {

	using namespace byteswap;
//==========================================================================
//
//
//
//==========================================================================

struct dpackfile_t
{
	char	name[56];
	uint32_t		filepos, filelen;
} ;

struct dpackheader_t
{
	uint32_t		ident;		// == IDPAKHEADER
	uint32_t		dirofs;
	uint32_t		dirlen;
} ;


//==========================================================================
//
// Open it
//
//==========================================================================

static bool OpenPak(FResourceFile* file, LumpFilterInfo* filter)
{
	dpackheader_t header;

	auto Reader = file->GetContainerReader();
	Reader->Read(&header, sizeof(header));
	uint32_t NumLumps = header.dirlen / sizeof(dpackfile_t);
	auto Entries = file->AllocateEntries(NumLumps);
	header.dirofs = LittleLong(header.dirofs);

	Reader->Seek (header.dirofs, FileReader::SeekSet);
	auto fd = Reader->Read (NumLumps * sizeof(dpackfile_t));
	auto fileinfo = (const dpackfile_t*)fd.data();

	for(uint32_t i = 0; i < NumLumps; i++)
	{
		Entries[i].Position = LittleLong(fileinfo[i].filepos);
		Entries[i].CompressedSize = Entries[i].Length = LittleLong(fileinfo[i].filelen);
		Entries[i].Flags = RESFF_FULLPATH;
		Entries[i].Namespace = ns_global;
		Entries[i].ResourceID = -1;
		Entries[i].Method = METHOD_STORED;
		Entries[i].FileName = file->NormalizeFileName(fileinfo[i].name);
	}
	file->GenerateHash();
	file->PostProcessArchive(filter);
	return true;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckPak(const char *filename, FileReader &file, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
	char head[4];

	if (file.GetLength() >= 12)
	{
		file.Seek(0, FileReader::SeekSet);
		file.Read(&head, 4);
		file.Seek(0, FileReader::SeekSet);
		if (!memcmp(head, "PACK", 4))
		{
			auto rf = new FResourceFile(filename, file, sp);
			if (OpenPak(rf, filter)) return rf;
			file = rf->Destroy();
		}
	}
	return NULL;
}

}
