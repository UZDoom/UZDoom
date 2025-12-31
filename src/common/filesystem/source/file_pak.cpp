/*
** file_pak.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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
