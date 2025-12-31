/*
** file_hog.cpp
**
** reads Descent .hog files
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




static bool OpenHog(FResourceFile* rf, LumpFilterInfo* filter)
{
    auto Reader = rf->GetContainerReader();
    FileReader::Size length = Reader->GetLength();

    std::vector<FResourceEntry> entries;
    // Hogs store their data as a list of file records, each containing a name, length and the actual data.
    // To read the directory the entire file must be scanned.
    while (Reader->Tell() <= length)
    {
        char name[13];

        auto r = Reader->Read(&name, 13);
        if (r < 13) break;
        name[12] = 0;
        uint32_t elength = Reader->ReadUInt32();

        FResourceEntry Entry;
        Entry.Position = Reader->Tell();
        Entry.CompressedSize = Entry.Length = elength;
        Entry.Flags = 0;
        Entry.CRC32 = 0;
        Entry.Namespace = ns_global;
        Entry.ResourceID = -1;
        Entry.Method = METHOD_STORED;
        Entry.FileName = rf->NormalizeFileName(name);
        entries.push_back(Entry);
        Reader->Seek(elength, FileReader::SeekCur);
    }
    auto Entries = rf->AllocateEntries((int)entries.size());
    memcpy(Entries, entries.data(), entries.size() * sizeof(Entries[0]));
    rf->GenerateHash();
    return true;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile* CheckHog(const char* filename, FileReader& file, LumpFilterInfo* filter, FileSystemMessageFunc Printf, StringPool* sp)
{
    char head[3];

    if (file.GetLength() >= 20)
    {
        file.Seek(0, FileReader::SeekSet);
        file.Read(&head, 3);
        if (!memcmp(head, "DHF", 3))
        {
            auto rf = new FResourceFile(filename, file, sp);
            if (OpenHog(rf, filter)) return rf;
            file = rf->Destroy();
        }
        file.Seek(0, FileReader::SeekSet);
    }
    return nullptr;
}


}
