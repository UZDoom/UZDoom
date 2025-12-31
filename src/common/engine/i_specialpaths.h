/*
** i_specialpaths.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2016 Marisa Heit
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

#include "zstring.h"

#if defined(__unix__) || defined(__HAIKU__)
FString GetUserFile (const char *path);
const char * GetConfigPath();
const char * GetCachePath();
const char * GetDataPath();
#endif

FString M_GetAppDataPath(bool create);
FString M_GetCachePath(bool create);
FString M_GetAutoexecPath();
FString M_GetConfigPath(bool for_reading);
FString M_GetScreenshotsPath();
FString M_GetSavegamesPath();
FString M_GetDocumentsPath();
FString M_GetDemoPath();

FString M_GetNormalizedPath(const char* path);

#ifdef __APPLE__
FString M_GetMacAppSupportPath(const bool create = true);
void M_GetMacSearchDirectories(FString& user_docs, FString& user_app_support, FString& local_app_support);
#endif // __APPLE__
