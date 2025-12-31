/*
** d_steam.cpp
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

#include "cmdlib.h"
#include "d_steam.h"
#include "engineerrors.h"
#include "i_system.h"
#include "sc_man.h"

static inline constexpr struct SteamAppInfo
{
	const char* const BasePath;
	const int AppID;
} SteamAppInfoList[] = {
	{"Doom 2/base", 2300},
	{"Final Doom/base", 2290},
	{"Heretic Shadow of the Serpent Riders/base", 2390},
	{"Hexen/base", 2360},
	{"Hexen Deathkings of the Dark Citadel/base", 2370},
	{"Ultimate Doom/base", 2280},
	{"Ultimate Doom/base/doom2", 2280},
	{"Ultimate Doom/base/tnt", 2280},
	{"Ultimate Doom/base/plutonia", 2280},
	{"DOOM 3 BFG Edition/base/wads", 208200},
	{"Strife", 317040},
	{"Ultimate Doom/rerelease/DOOM_Data/StreamingAssets", 2280},
	{"Ultimate Doom/rerelease", 2280},
	{"Doom 2/rerelease/DOOM II_Data/StreamingAssets", 2300},
	{"Doom 2/finaldoombase", 2300},
    {"Master Levels of Doom/doom2", 9160},
	{"Heretic + Hexen/dos/base/heretic", 3286930},
	{"Heretic + Hexen/dos/base/hexen", 3286930},
	{"Heretic + Hexen/dos/base/hexendk", 3286930}
};

static void PSR_FindEndBlock(FScanner &sc)
{
	int depth = 1;
	do
	{
		if(sc.CheckToken('}'))
			--depth;
		else if(sc.CheckToken('{'))
			++depth;
		else
			sc.MustGetAnyToken();
	}
	while(depth);
}

TArray<FString> D_ParseSteamRegistry(const char* path)
{
	TArray<FString> result;
	FScanner sc;
	if (sc.OpenFile(path))
	{
		sc.SetCMode(true);

		sc.MustGetToken(TK_StringConst);
		sc.MustGetToken('{');
		// Get a list of possible install directories.
		while(sc.GetToken() && sc.TokenType != '}')
		{
			sc.TokenMustBe(TK_StringConst);
			sc.MustGetToken('{');

			while(sc.GetToken() && sc.TokenType != '}')
			{
				sc.TokenMustBe(TK_StringConst);
				FString key(sc.String);
				if(key.CompareNoCase("path") == 0)
				{
					sc.MustGetToken(TK_StringConst);
					result.Push(FString(sc.String) + "/steamapps/common");
					PSR_FindEndBlock(sc);
					break;
				}
				else if(sc.CheckToken('{'))
				{
					PSR_FindEndBlock(sc);
				}
				else
				{
					sc.MustGetToken(TK_StringConst);
				}
			}
		}
	}
	return result;
}

TArray<FString> D_GetSteamGamePaths()
{
	TArray<FString> result;

	// Get the install location of Steam on our system
	FString SteamPath = I_GetSteamPath();
	if (SteamPath.IsEmpty() || !DirExists(SteamPath.GetChars()))
	{
		// Steam is not installed
		return result;
	}

	// Parse libraryfolders.vdf to figure out where the user has
	// their games installed to.
	TArray<FString> SteamLibraryFolders;
	try
	{
		FString RegPath = SteamPath + "/config/libraryfolders.vdf";
		SteamLibraryFolders = D_ParseSteamRegistry(RegPath.GetChars());
	}
	catch (const CRecoverableError &error)
	{
		// If we can't parse for some reason just pretend we can't find anything.
		return result;
	}

	for (FString& folder : SteamLibraryFolders)
	{
		folder.ReplaceChars('\\', '/');
		folder += "/";
	}

	// Always add the "canon" Steam library path, just in case
	// libraryfolders.vdf does not exist.
	SteamLibraryFolders.Push(SteamPath + "/steamapps/common/");

	for (unsigned int i = 0; i < std::size(SteamAppInfoList); ++i)
	{
		for (const FString& folder : SteamLibraryFolders)
		{
			FString candidate(folder + SteamAppInfoList[i].BasePath);

			if (DirExists(candidate.GetChars()))
			{
				result.Push(candidate);
			}
		}
	}

	return result;
}
