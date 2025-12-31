/*
** file_lump.cpp
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
//==========================================================================
//
// Open it
//
//==========================================================================

static bool OpenLump(FResourceFile* file, LumpFilterInfo*)
{
	auto Entries = file->AllocateEntries(1);
	Entries[0].FileName = file->NormalizeFileName(ExtractBaseName(file->GetFileName(), true).c_str());
	Entries[0].Namespace = ns_global;
	Entries[0].ResourceID = -1;
	Entries[0].Position = 0;
	Entries[0].CompressedSize = Entries[0].Length = file->GetContainerReader()->GetLength();
	Entries[0].Method = METHOD_STORED;
	Entries[0].Flags = 0;
	return true;
}

//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckLump(const char *filename, FileReader &file, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
	// always succeeds
	auto rf = new FResourceFile(filename, file, sp);
	if (OpenLump(rf, filter)) return rf;
	file = rf->Destroy();
	return NULL;
}

}
