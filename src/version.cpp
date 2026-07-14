/*
** version.cpp
**
** Functions to get build info
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

#include "gitinfo.h"
#include "version.h"
#include "basics.h"

//==========================================================================
//
// <Tag>-<Distance>-g<commit>
//
//==========================================================================

const char *GetVersionString()
{
	return (GIT_DESCRIPTION[0] == '\0')? VERSIONSTR: GIT_DESCRIPTION;
}

//==========================================================================
//
// <commit>
//
//==========================================================================

const char *GetGitHash()
{
	return GIT_HASH;
}

//==========================================================================
//
// ISO 8601
//
//==========================================================================

const char *GetGitTime()
{
	return GIT_TIME;
}

//==========================================================================
//
// Closest git tag
//
//==========================================================================

const char *GetGitTag()
{
	return GIT_TAG;
}

//==========================================================================
//
// Distance to closest git tag
//
//==========================================================================

int GetGitDistance()
{
	return GIT_DISTANCE;
}

VersionInfo GetCurrentVersion()
{
	return MakeVersion(VER_MAJOR, VER_MINOR, VER_REVISION);
}

VersionInfo GetCurrentVersionForUpdate(UpdateChannel channel)
{
#ifdef DEBUG_FORCE_UPDATE
	return VersionInfo(1,0,0,0);
#endif

	switch(channel)
	{
	case UpdateChannel::STABLE:
	case UpdateChannel::RELEASE_CANDIDATE:
		return VersionInfo(VER_MAJOR, VER_MINOR, VER_REVISION, RC_REVISION);
	case UpdateChannel::PREVIEW:
	case UpdateChannel::TESTING:
		return VersionInfo(VER_MAJOR, VER_MINOR, VER_REVISION, GIT_DISTANCE);
	}
}

VersionInfo GetCurrentEngineVersion()
{
	return MakeVersion(ENG_MAJOR, ENG_MINOR, ENG_REVISION);
}
