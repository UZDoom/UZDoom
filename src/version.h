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

#pragma once

/** Lots of different version numbers **/

#define VERSIONSTR "5.0.0-pre"

// The version as seen in the Windows resource
#define RC_FILEVERSION 4,9999,9999,0
#define RC_PRODUCTVERSION 4,9999,9999,0
#define RC_PRODUCTVERSION2 VERSIONSTR
// These are for content versioning.
#define VER_MAJOR 5
#define VER_MINOR 0
#define VER_REVISION 0

// This should always refer to the UZDoom version a derived port is based on and not reflect the derived port's version number!
#define ENG_MAJOR 5
#define ENG_MINOR 0
#define ENG_REVISION 0

// Version stored in the ini's [LastRun] section.
// Bump it if you made some configuration change that you want to
// be able to migrate in FGameConfigFile::DoGlobalSetup().
#define ENGINELASTRUNVERSION "232"
#define GAMELASTRUNVERSION "1"

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
#define BUGS_URL "https://github.com/UZDoom/UZDoom/issues"

#define UPDATER_URL_STABLE "https://zdoom.org/uzdoom-latest.php"
#define UPDATER_URL_STABLE_BACKUP "https://api.github.com/repos/UZDoom/UZDoom/releases/latest"

#define UPDATER_URL_PREVIEW "https://zdoom.org/uzdoom-preview.php"
#define UPDATER_URL_PREVIEW_BACKUP "https://api.github.com/repos/UZDoom/UZDoom/releases/tags/x-preview"

#define UPDATER_URL_TESTING "https://zdoom.org/uzdoom-testing.php"
#define UPDATER_URL_TESTING_BACKUP "https://api.github.com/repos/UZDoom/UZDoom/releases/tags/x-testing"

#define UPDATER_URL_ALL "https://zdoom.org/uzdoom-all.php"
#define UPDATER_URL_ALL_BACKUP "https://api.github.com/repos/UZDoom/UZDoom/releases"

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

const char *GetVersionString();
const char *GetGitHash();
const char *GetGitTime();
const char *GetGitTag();
int GetGitDistance();

#define RC_REVISION_NOTRC 999999

#ifdef __cplusplus

#include <compare>
#include <cstdint>

class FString;

enum class UpdateChannel
{
	STABLE,
	PREVIEW,
	TESTING,
	RELEASE_CANDIDATE,
};

#define CURRENT_UPDATE_CHANNEL UpdateChannel::PREVIEW

#define RC_REVISION 999999

struct VersionInfo
{
	uint16_t major;
	uint16_t minor;
	uint32_t revision;
	uint32_t distance;

	constexpr VersionInfo() = default;
	constexpr VersionInfo(uint16_t _major, uint16_t _minor, uint32_t _revision = 0, uint32_t _distance = 0)
		: major(_major), minor(_minor), revision(_revision), distance(_distance)
	{
	}
	explicit VersionInfo(const char *);

	constexpr bool operator <=(const VersionInfo& o) const
	{
		return operator<=>(o) != std::strong_ordering::greater;
	}
	constexpr bool operator >=(const VersionInfo& o) const
	{
		return operator<=>(o) != std::strong_ordering::less;
	}
	constexpr bool operator > (const VersionInfo& o) const
	{
		return operator<=>(o) == std::strong_ordering::greater;
	}
	constexpr bool operator < (const VersionInfo& o) const
	{
		return operator<=>(o) == std::strong_ordering::less;
	}
	constexpr bool operator == (const VersionInfo& o) const
	{
		return operator<=>(o) == std::strong_ordering::equal;
	}
	constexpr bool operator != (const VersionInfo& o) const
	{
		return operator<=>(o) != std::strong_ordering::equal;
	}

	constexpr std::strong_ordering operator <=> (const VersionInfo& o) const
	{
		if(major != o.major)
		{
			return major <=> o.major;
		}
		else if(minor != o.minor)
		{
			return minor <=> o.minor;
		}
		else if(revision != o.revision)
		{
			return revision <=> o.revision;
		}
		else //if(distance != o.distance)
		{
			return distance <=> o.distance;
		}
	}

	void operator=(const char* string);
	explicit operator FString();
};

// Cannot be a constructor because Lemon would puke on it.
constexpr VersionInfo MakeVersion(unsigned int ma, unsigned int mi, unsigned int re = 0)
{
	return{ (uint16_t)ma, (uint16_t)mi, (uint32_t)re };
}

VersionInfo GetCurrentVersion();

VersionInfo GetCurrentVersionForUpdate(UpdateChannel channel);

VersionInfo GetCurrentEngineVersion();

#endif // __cplusplus
