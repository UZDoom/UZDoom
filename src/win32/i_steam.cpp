/*
** i_steam.cpp
**
** Detection for IWADs installed by Steam (or other distributors)
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2007-2012 Skulltag Development Team
** Copyright 2007-2016 Zandronum Development Team
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: LicenseRef-Almost-Sleepycat
**
**---------------------------------------------------------------------------
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <map>

#include <stdarg.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <richedit.h>
#include <wincrypt.h>
#include <shlwapi.h>

#include "printf.h"

#include "engineerrors.h"
#include "version.h"
#include "i_sound.h"
#include "stats.h"
#include "v_text.h"
#include "utf8.h"

#include "d_main.h"
#include "d_net.h"
#include "g_game.h"
#include "c_dispatch.h"

#include "gameconfigfile.h"
#include "v_font.h"
#include "g_level.h"
#include "doomstat.h"
#include "bitmap.h"
#include "cmdlib.h"
#include "i_interface.h"

//==========================================================================
//
// QueryPathKey
//
// Returns the value of a registry key into the output variable value.
//
//==========================================================================

static bool QueryPathKey(HKEY key, const wchar_t *keypath, const wchar_t *valname, FString &value)
{
	HKEY pathkey;
	DWORD pathtype;
	DWORD pathlen;
	LONG res;

	value = "";
	if(ERROR_SUCCESS == RegOpenKeyEx(key, keypath, 0, KEY_QUERY_VALUE, &pathkey))
	{
		if (ERROR_SUCCESS == RegQueryValueEx(pathkey, valname, 0, &pathtype, NULL, &pathlen) &&
			pathtype == REG_SZ && pathlen != 0)
		{
			// Don't include terminating null in count
			TArray<wchar_t> chars(pathlen + 1, true);
			res = RegQueryValueEx(pathkey, valname, 0, NULL, (LPBYTE)chars.Data(), &pathlen);
			if (res == ERROR_SUCCESS) value = FString(chars.Data());
		}
		RegCloseKey(pathkey);
	}
	return value.IsNotEmpty();
}

//==========================================================================
//
// I_GetGogPaths
//
// Check the registry for GOG installation paths, so we can search for IWADs
// that were bought from GOG.com. This is a bit different from the Steam
// version because each game has its own independent installation path, no
// such thing as <steamdir>/SteamApps/common/<GameName>.
//
//==========================================================================

TArray<FString> I_GetGogPaths()
{
	TArray<FString> result;
	FString path;
	std::wstring gamepath;

#ifdef _WIN64
	std::wstring gogregistrypath = L"Software\\Wow6432Node\\GOG.com\\Games";
#else
	// If a 32-bit ZDoom runs on a 64-bit Windows, this will be transparently and
	// automatically redirected to the Wow6432Node address instead, so this address
	// should be safe to use in all cases.
	std::wstring gogregistrypath = L"Software\\GOG.com\\Games";
#endif

	// Look for Ultimate Doom
	gamepath = gogregistrypath + L"\\1435827232";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path);	// directly in install folder
	}

	// Look for Doom I Enhanced
	gamepath = gogregistrypath + L"\\2015545325";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path + "/DOOM_Data/StreamingAssets");	// in a subdirectory
	}

	// Look for Doom II
	gamepath = gogregistrypath + L"\\1435848814";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path + "/doom2");	// in a subdirectory
		// If direct support for the Master Levels is ever added, they are in path + /master/wads
	}

	// Look for Doom II Enhanced
	gamepath = gogregistrypath + L"\\1426071866";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path + "/DOOM II_Data/StreamingAssets");	// in a subdirectory
	}

	// Look for Doom + Doom II
	gamepath = gogregistrypath + L"\\1413291984";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path);	// directly in install folder
	}

	// Look for Final Doom
	gamepath = gogregistrypath + L"\\1435848742";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		// in subdirectories
		result.Push(path + "/TNT");
		result.Push(path + "/Plutonia");
	}

	// Look for Doom 3: BFG Edition
	gamepath = gogregistrypath + L"\\1135892318";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path + "/base/wads");	// in a subdirectory
	}

	// Look for Strife: Veteran Edition
	gamepath = gogregistrypath + L"\\1432899949";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path);	// directly in install folder
	}

	// Look for Heretic: SOTSR
	gamepath = gogregistrypath + L"\\1290366318";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path);	// directly in install folder
	}

	// Look for Hexen: Beyond Heretic
	gamepath = gogregistrypath + L"\\1247951670";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path);	// directly in install folder
	}

	// Look for Hexen: Death Kings
	gamepath = gogregistrypath + L"\\1983497091";
	if (QueryPathKey(HKEY_LOCAL_MACHINE, gamepath.c_str(), L"Path", path))
	{
		result.Push(path);	// directly in install folder
	}

	return result;
}

//==========================================================================
//
// I_GetSteamPath
//
// Check the registry for the path to Steam, so that we can search for
// IWADs that were bought with Steam.
//
//==========================================================================

FString I_GetSteamPath()
{
	FString SteamPath;

	if (!QueryPathKey(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", SteamPath))
	{
		if (!QueryPathKey(HKEY_LOCAL_MACHINE, L"Software\\Valve\\Steam", L"InstallPath", SteamPath))
			return "";
	}

	return SteamPath;
}

//==========================================================================
//
// I_GetBethesdaPath
//
// Check the registry for the path to the Bethesda.net Launcher, so that we
// can search for IWADs that were bought from Bethesda.net.
//
//==========================================================================

TArray<FString> I_GetBethesdaPath()
{
	TArray<FString> result;
	static const char* const bethesda_dirs[] =
	{
		"DOOM_Classic_2019/base",
		"DOOM_Classic_2019/rerelease/DOOM_Data/StreamingAssets",
		"DOOM_II_Classic_2019/base",
		"DOOM_II_Classic_2019/rerelease/DOOM II_Data/StreamingAssets",
		"DOOM 3 BFG Edition/base/wads",
		"Heretic Shadow of the Serpent Riders/base",
		"Hexen/base",
		"Hexen Deathkings of the Dark Citadel/base"

		// Alternate DOS versions of Doom and Doom II (referred to as "Original" in the
		// Bethesda Launcher). While the DOS versions that come with the Unity ports are
		// unaltered, these use WADs from the European PSN versions. These WADs are currently
		// misdetected by GZDoom: DOOM.WAD is detected as the Unity version (which it's not),
		// while DOOM2.WAD is detected as the original DOS release despite having Doom 3: BFG
		// Edition's censored secret level titles (albeit only in the title patches, not in
		// the automap). Unfortunately, these WADs have exactly the same lump names as the WADs
		// they're misdetected as, so it's not currently possible to distinguish them using
		// GZDoom's current IWAD detection system. To prevent them from possibly overriding the
		// real Unity DOOM.WAD and DOS DOOM2.WAD, these paths have been commented out.
		//"Ultimate DOOM/base",
		//"DOOM II/base",

		// Doom Eternal includes DOOM.WAD and DOOM2.WAD, but they're the same misdetected
		// PSN versions used by the alternate DOS releases above.
		//"Doom Eternal/base/classicwads"
	};

#ifdef _WIN64
	const wchar_t *bethesdaregistrypath = L"Software\\Wow6432Node\\Bethesda Softworks\\Bethesda.net";
#else
	// If a 32-bit ZDoom runs on a 64-bit Windows, this will be transparently and
	// automatically redirected to the Wow6432Node address instead, so this address
	// should be safe to use in all cases.
	const wchar_t *bethesdaregistrypath = L"Software\\Bethesda Softworks\\Bethesda.net";
#endif

	FString path;
	if (!QueryPathKey(HKEY_LOCAL_MACHINE, bethesdaregistrypath, L"installLocation", path))
	{
		return result;
	}
	path += "/games/";

	for (unsigned int i = 0; i < countof(bethesda_dirs); ++i)
	{
		result.Push(path + bethesda_dirs[i]);
	}

	return result;
}
