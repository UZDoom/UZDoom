/*
** i_steam.cpp
**
** Detection for IWADs installed by Steam (or other distributors)
**
**---------------------------------------------------------------------------
***
** Copyright 2013 Braden Obrzut
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
**
*/

#include <sys/stat.h>

#ifdef __APPLE__
#include "m_misc.h"
#endif // __APPLE__

#include "d_main.h"
#include "engineerrors.h"
#include "sc_man.h"
#include "cmdlib.h"

FString I_GetSteamPath()
{
#ifdef __APPLE__
	return M_GetMacAppSupportPath() + "/Steam";
#else
	char* home = getenv("HOME");
	if (home != NULL && *home != '\0')
	{
		FString regPath;
		regPath.Format("%s/.local/share/Steam", home);
		return regPath;
	}
	return "";
#endif
}

TArray<FString> I_GetGogPaths()
{
	// GOG's Doom games are Windows only at the moment
	return TArray<FString>();
}
