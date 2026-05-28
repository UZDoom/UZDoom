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
