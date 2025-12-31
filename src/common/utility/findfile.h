/*
** findfile.h
**
** Wrapper around the native directory scanning API
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

#pragma once
// Directory searching routines

#include <stdint.h>
#include <string>
#include <vector>

class FConfigFile;

enum EFileRequirements
{
    REQUIRE_NONE    = 0,
    REQUIRE_IWAD    = 1,
    REQUIRE_FILE    = 2,
    REQUIRE_OPTFILE = 4,

    REQUIRE_DEFAULT = REQUIRE_IWAD|REQUIRE_FILE,
    REQUIRE_ALL = REQUIRE_IWAD|REQUIRE_FILE|REQUIRE_OPTFILE
};

bool D_AddFile(std::vector<std::string>& wadfiles, const char* file, bool check, int position, FConfigFile* config, bool optional = false);
void D_AddWildFile(std::vector<std::string>& wadfiles, const char* value, const char *extension, FConfigFile* config, bool optional = false);
void D_AddConfigFiles(std::vector<std::string>& wadfiles, const char* section, const char* extension, FConfigFile* config, bool optional = false);
void D_AddDirectory(std::vector<std::string>& wadfiles, const char* dir, const char *filespec, FConfigFile* config, bool optional = false);
const char* BaseFileSearch(const char* file, const char* ext, bool lookfirstinprogdir, FConfigFile* config);
void D_FileNotFound(EFileRequirements test, const char* type, const char* file);
