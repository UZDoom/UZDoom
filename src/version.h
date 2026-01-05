/*
** version.h
**
** ZDoom version constants
**
**---------------------------------------------------------------------------
**
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
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

#ifndef __VERSION_H__
#define __VERSION_H__

const char *GetGitDescription();
const char *GetGitHash();
const char *GetGitTime();
const char *GetVersionString();

/** Lots of different version numbers **/

#define VERSIONSTR "4.15pre"

// The version as seen in the Windows resource
#define RC_FILEVERSION 4,14,9999,0
#define RC_PRODUCTVERSION 4,14,9999,0
#define RC_PRODUCTVERSION2 VERSIONSTR
// These are for content versioning.
#define VER_MAJOR 4
#define VER_MINOR 15
#define VER_REVISION 1

// This should always refer to the UZDoom version a derived port is based on and not reflect the derived port's version number!
#define ENG_MAJOR 4
#define ENG_MINOR 15
#define ENG_REVISION 1

// Version stored in the ini's [LastRun] section.
// Bump it if you made some configuration change that you want to
// be able to migrate in FGameConfigFile::DoGlobalSetup().
#define LASTRUNVERSION "229"

// Protocol version used in demos.
// Bump it if you change existing DEM_ commands or add new ones.
// Otherwise, it should be safe to leave it alone.
#define DEMOGAMEVERSION 0x221

// Minimum demo version we can play.
// Bump it whenever you change or remove existing DEM_ commands.
#define MINDEMOVERSION 0x221

// SAVEVER is the version of the information stored in level snapshots.
// Note that SAVEVER is not directly comparable to VERSION.
// SAVESIG should match SAVEVER.

// extension for savegames
#define SAVEGAME_EXT "zds"

// MINSAVEVER is the minimum level snapshot version that can be loaded.
#define MINSAVEVER 4556

// Use 4500 as the base git save version, since it's higher than the
// SVN revision ever got.
#define SAVEVER 4560

// This is so that derivates can use the same savegame versions without worrying about engine compatibility
#define GAMESIG "UZDOOM"

// list of compatible ports, ex.:
// #define ALLOWLOADIN "PORT1", "PORT2", "PORT3"
#define ALLOWLOADIN "LZDOOM"

#ifndef LOAD_GZDOOM_4142_SAVES
    #define LOAD_GZDOOM_4142_SAVES 1
#endif

#define BASEWAD "uzdoom.pk3"
// Set OPTIONALWAD to "" (null) to disable searching for it
#define OPTIONALWAD "game_support.pk3"
#define GZDOOM 1
#define VR3D_ENABLED

// More stuff that needs to be different for derivatives.
#define GAMENAME "UZDoom"
#define WGAMENAME L"UZDoom"
#define GAMENAMELOWERCASE "uzdoom"
#define APPID "org.zdoom.UZDoom"
#define QUERYIWADDEFAULT true
#define FORUM_URL "http://forum.zdoom.org/"
#define BUGS_FORUM_URL	"http://forum.zdoom.org/viewforum.php?f=2"
// For QUERYIWADDEFAULT: Set to 'true' to always show dialog box on startup by default, 'false' to disable.
// Should set to 'false' for standalone games, and set to 'true' for regular source port forks that are meant to run any game.

#if defined(__APPLE__) || defined(_WIN32)
#define GAME_DIR GAMENAME
#elif defined(__HAIKU__)
#define GAME_DIR "config/settings/" GAMENAME
#endif

#define DEFAULT_DISCORD_APP_ID "1428620310302691349"

const int SAVEPICWIDTH = 216;
const int SAVEPICHEIGHT = 162;
const int VID_MIN_WIDTH = 320;
const int VID_MIN_HEIGHT = 200;

#endif //__VERSION_H__
