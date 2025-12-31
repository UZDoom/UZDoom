/*
** i_specialpaths.cpp
**
** Gets special system folders where data should be stored. (Unix version)
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2016 Marisa Heit
** Copyright 2016 Christoph Oelckers
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

#include <sys/stat.h>
#include <sys/types.h>

#include "cmdlib.h"
#include "engineerrors.h"
#include "printf.h"
#include "version.h"
#include "zstring.h"

extern bool netgame;

#ifdef __APPLE
#define DEFGETPATH(name, var, fallback) \
	const char * Get##name##Path()      \
	{                                   \
		return fallback;                \
	}
#else
#define DEFGETPATH(name, var, fallback) \
	const char * Get##name##Path()      \
	{                                   \
		static const char *path;        \
		static bool tested;             \
		if (!tested)                    \
		{                               \
			path = getenv(var);         \
			tested = true;              \
			if (!path) path = fallback; \
		}                               \
		return path;                    \
	}
#endif
#ifdef __HAIKU__
DEFGETPATH(Config, "XDG_CONFIG_HOME", "$HOME/config/settings");
DEFGETPATH(Cache, "XDG_CACHE_HOME", "$HOME/config/cache");
DEFGETPATH(Data, "XDG_DATA_HOME", "$HOME/config/non-packaged/data");
#else
DEFGETPATH(Config, "XDG_CONFIG_HOME", "$HOME/.config");
DEFGETPATH(Cache, "XDG_CACHE_HOME", "$HOME/.cache");
DEFGETPATH(Data, "XDG_DATA_HOME", "$HOME/.local/share");
DEFGETPATH(Pictures, "XDG_PICTURES_DIR", "$HOME/Pictures");
#endif
#undef DEFGETPATH

FString GetUserFile (const char *file)
{
	struct stat info;

	FString path = FStringf("%s/" GAMENAMELOWERCASE "/", GetConfigPath());
	path = NicePath(path.GetChars());

	if (stat (path.GetChars(), &info) == -1)
	{
		struct stat extrainfo;

		// Sanity check for $HOME/.config
		FString configPath = NicePath(GetConfigPath());
		if (stat (configPath.GetChars(), &extrainfo) == -1)
		{
			if (mkdir (configPath.GetChars(), S_IRUSR | S_IWUSR | S_IXUSR) == -1)
			{
				I_FatalError ("Failed to create %s directory:\n%s", GetConfigPath(), strerror(errno));
			}
		}
		else if (!S_ISDIR(extrainfo.st_mode))
		{
			I_FatalError ("%s must be a directory", GetConfigPath());
		}

		if (mkdir (path.GetChars(), S_IRUSR | S_IWUSR | S_IXUSR) == -1)
		{
			I_FatalError ("Failed to create %s directory:\n%s", path.GetChars(), strerror (errno));
		}
	}
	else
	{
		if (!S_ISDIR(info.st_mode))
		{
			I_FatalError ("%s must be a directory", path.GetChars());
		}
	}
	path += file;
	return path;
}

//===========================================================================
//
// M_GetAppDataPath														Unix
//
// Returns the path for the AppData folder.
//
//===========================================================================

FString M_GetAppDataPath(bool create)
{
	static FString path = FStringf("%s/games/" GAMENAMELOWERCASE, GetDataPath());
	path = NicePath(path.GetChars());

	if (create)
	{
		CreatePath(path.GetChars());
	}
	return path;
}

//===========================================================================
//
// M_GetCachePath														Unix
//
// Returns the path for cache GL nodes.
//
//===========================================================================

FString M_GetCachePath(bool create)
{
	static FString path = FStringf("%s/" GAMENAMELOWERCASE, GetCachePath());
	path = NicePath(path.GetChars());

	if (create)
	{
		CreatePath(path.GetChars());
	}
	return path;
}

//===========================================================================
//
// M_GetAutoexecPath													Unix
//
// Returns the expected location of autoexec.cfg.
//
//===========================================================================

FString M_GetAutoexecPath()
{
	return GetUserFile("autoexec.cfg");
}

//===========================================================================
//
// M_GetConfigPath														Unix
//
// Returns the path to the config file. On Windows, this can vary for reading
// vs writing. i.e. If $PROGDIR/zdoom-<user>.ini does not exist, it will try
// to read from $PROGDIR/zdoom.ini, but it will never write to zdoom.ini.
//
//===========================================================================

FString M_GetConfigPath(bool for_reading)
{
	return GetUserFile(GAMENAMELOWERCASE ".ini");
}

//===========================================================================
//
// M_GetDocumentsPath												Unix
//
// Returns the path to the default documents directory.
//
//===========================================================================

FString M_GetDocumentsPath()
{
#ifdef __HAIKU__
	return FStringf("%s/" GAMENAMELOWERCASE "/", GetConfigPath());
#else
	return M_GetAppDataPath(false) + "/";
#endif
}


//===========================================================================
//
// M_GetScreenshotsPath													Unix
//
// Returns the path to the default screenshots directory.
//
//===========================================================================

FString M_GetScreenshotsPath()
{
#ifdef __HAIKU__
	static FString path = M_GetDocumentsPath() + "screenshots";
#else
	static FString path = FStringf("%s/Screenshots/" GAMENAME, GetPicturesPath());
#endif
	path = NicePath(path.GetChars());
	return path;
}

//===========================================================================
//
// M_GetSavegamesPath													Unix
//
// Returns the path to the default save games directory.
//
//===========================================================================

FString M_GetSavegamesPath()
{
	if (netgame)
		return M_GetDocumentsPath() + "savegames/netgame/";
	else
		return M_GetDocumentsPath() + "savegames/";
}

//===========================================================================
//
// M_GetDemoPath													Unix
//
// Returns the path to the default demo directory.
//
//===========================================================================

FString M_GetDemoPath()
{
	return M_GetDocumentsPath() + "demo/";
}

//===========================================================================
//
// M_NormalizedPath
//
// Normalizes the given path and returns the result.
//
//===========================================================================

FString M_GetNormalizedPath(const char* path)
{
	char *actualpath;
	actualpath = realpath(path, NULL);
	if (!actualpath) // error ?
		return nullptr;
	FString fullpath = actualpath;
	// free(actualpath); // this needs to be freed, I think
	return fullpath;
}
