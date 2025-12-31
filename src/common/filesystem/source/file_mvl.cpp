/*
** file_mvl.cpp
**
** reads Descent2 .mvl files
**
**---------------------------------------------------------------------------
**
** Copyright 2023 Christoph Oelckers
** Copyright 2023-2025 GZDoom Maintainers and Contributors
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



static bool OpenMvl(FResourceFile* rf, LumpFilterInfo* filter)
{
    auto Reader = rf->GetContainerReader();
    auto count = Reader->ReadUInt32();
    auto Entries = rf->AllocateEntries(count);
    size_t pos = 8 + (17 * count);   // files start after the directory

    for (uint32_t i = 0; i < count; i++)
    {
        char name[13];
        Reader->Read(&name, 13);
        name[12] = 0;
        uint32_t elength = Reader->ReadUInt32();

        Entries[i].Position = pos;
        Entries[i].CompressedSize = Entries[i].Length = elength;
        Entries[i].ResourceID = -1;
        Entries[i].FileName = rf->NormalizeFileName(name);

        pos += elength;
    }

    return true;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile* CheckMvl(const char* filename, FileReader& file, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
    char head[4];

    if (file.GetLength() >= 20)
    {
        file.Seek(0, FileReader::SeekSet);
        file.Read(&head, 4);
        if (!memcmp(head, "DMVL", 4))
        {
            auto rf = new FResourceFile(filename, file, sp);
            if (OpenMvl(rf, filter)) return rf;
            file = rf->Destroy();
        }
        file.Seek(0, FileReader::SeekSet);
    }
    return nullptr;
}


}
