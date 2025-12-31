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
** 4. Redistributions in any form must be accompanied by information on how to
**    obtain complete source code for the software and any accompanying software
**    that uses the software. The source code must either be included in the
**    distribution or be available for no more than the cost of distribution plus
**    a nominal fee, and must be freely redistributable under reasonable
**    conditions. For an executable file, complete source code means the source
**    code for all modules it contains. It does not include source code for
**    modules or files that typically accompany the major components of the
**    operating system on which the executable file runs.
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
