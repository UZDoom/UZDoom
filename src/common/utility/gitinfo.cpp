/*
** gitinfo.cpp
**
** Returns strings from gitinfo.h.
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2016 Marisa Heit
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
** This file is just here so that when gitinfo.h changes, only one source
** file needs to be recompiled.
*/

#include "gitinfo.h"
#include "version.h"

const char *GetGitDescription()
{
	return GIT_DESCRIPTION;
}

const char *GetGitHash()
{
	return GIT_HASH;
}

const char *GetGitTime()
{
	return GIT_TIME;
}

const char *GetVersionString()
{
	if (GetGitDescription()[0] == '\0')
	{
		return VERSIONSTR;
	}
	else
	{
		return GIT_DESCRIPTION;
	}
}
