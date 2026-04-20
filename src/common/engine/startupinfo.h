/*
** startupinfo.h
**
**
**
**---------------------------------------------------------------------------
**
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

#include <stdint.h>
#include "zstring.h"

struct FStartupInfo
{
	FString Name;
	uint32_t FgColor;			// Foreground color for title banner
	uint32_t BkColor;			// Background color for title banner
	FString Song;
	FString con;
	FString def;
	FString DiscordAppId = nullptr;
	FString SteamAppId = nullptr;
	int Type;
	int LoadLights = -1;
	int LoadBrightmaps = -1;
	int LoadWidescreen = -1;
	enum
	{
		DefaultStartup,
		DoomStartup,
		HereticStartup,
		HexenStartup,
		StrifeStartup,
	};
};


extern FStartupInfo GameStartupInfo;	

