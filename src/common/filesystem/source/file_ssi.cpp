/*
** file_ssi.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2020 Christoph Oelckers
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
// Note that SSIs can contain embedded GRPs which must be flagged accordingly.
//
//==========================================================================

static bool OpenSSI(FResourceFile* file, int version, int EntryCount, LumpFilterInfo*)
{
	uint32_t NumLumps = EntryCount * 2;
	auto Entries = file->AllocateEntries(NumLumps);
	auto Reader = file->GetContainerReader();
	

	int32_t j = (version == 2 ? 267 : 254) + (EntryCount * 121);
	for (uint32_t i = 0; i < NumLumps; i+=2)
	{
		char fn[13];
		int strlength = Reader->ReadUInt8();
		if (strlength > 12) strlength = 12;

		Reader->Read(fn, 12);
		fn[strlength] = 0;
		int flength = Reader->ReadInt32();

		Entries[i].Position = j;
		Entries[i].CompressedSize = Entries[i].Length = flength;
		Entries[i].Flags = 0;
		Entries[i].Namespace = ns_global;
		Entries[i].Method = METHOD_STORED;
		Entries[i].ResourceID = -1;
		Entries[i].FileName = file->NormalizeFileName(fn);
		if (strstr(fn, ".GRP")) Entries[i].Flags |= RESFF_EMBEDDED;

		// SSI files can swap the order of the extension's characters - but there's no reliable detection for this and it can be mixed inside the same container, 
		// so we have no choice but to create another file record for the altered name.
		std::swap(fn[strlength - 1], fn[strlength - 3]);

		Entries[i + 1].Position = j;
		Entries[i + 1].CompressedSize = Entries[i + 1].Length = flength;
		Entries[i + 1].Flags = 0;
		Entries[i + 1].Namespace = ns_global;
		Entries[i + 1].ResourceID = -1;
		Entries[i + 1].FileName = file->NormalizeFileName(fn);
		Entries[i + 1].Method = METHOD_STORED;
		if (strstr(fn, ".GRP")) Entries[i + 1].Flags |= RESFF_EMBEDDED;

		j += flength;

		Reader->Seek(104, FileReader::SeekCur);
		file->GenerateHash();
	}
	return true;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile* CheckSSI(const char* filename, FileReader& file, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
	char zerobuf[72];
	char buf[72];
	memset(zerobuf, 0, 72);

	auto skipstring = [&](size_t length)
	{
		size_t strlength = file.ReadUInt8();
		if (strlength > length) return false;
		size_t count = file.Read(buf, length);
		buf[length] = 0;
		if (count != length || strlen(buf) != strlength) return false;
		if (length != strlength && memcmp(buf + strlength, zerobuf, length - strlength)) return false;
		return true;
	};
	if (file.GetLength() >= 12)
	{
		// check if SSI
		// this performs several checks because there is no "SSI" magic
		int version = file.ReadInt32();
		if (version == 1 || version == 2) // if
		{
			int numfiles = file.ReadInt32();
			if (!skipstring(32)) return nullptr;
			if (version == 2 && !skipstring(12)) return nullptr;
			for (int i = 0; i < 3; i++)
			{
				if (!skipstring(70)) return nullptr;
			}
			auto ssi = new FResourceFile(filename, file, sp);
			if (OpenSSI(ssi, version, numfiles, filter)) return ssi;
			file = ssi->Destroy();
		}
	}
	return nullptr;
}

}
