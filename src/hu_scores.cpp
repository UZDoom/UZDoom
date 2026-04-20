/*
** hu_scores.cpp
**
** Routines for drawing the scoreboards.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2007-2008 Christopher Westley
** Copyright 2007-2016 Christoph Oelckers
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

// HEADER FILES ------------------------------------------------------------

#include "c_console.h"
#include "teaminfo.h"

#include "v_video.h"
#include "doomstat.h"
#include "g_level.h"
#include "d_netinf.h"
#include "v_font.h"
#include "d_player.h"
#include "hu_stuff.h"
#include "gstrings.h"
#include "d_net.h"
#include "c_dispatch.h"
#include "g_levellocals.h"
#include "g_game.h"
#include "sbar.h"
#include "texturemanager.h"
#include "v_draw.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void HU_DoDrawScores (player_t *, player_t *[MAXPLAYERS]);
static void HU_DrawTimeRemaining (int y);
static void HU_DrawPlayer(player_t *, bool, int, int, int, int, int, int, int, int, int);
static void HU_DrawColorBar(int x, int y, int height, int playernum);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

EXTERN_CVAR (Float, timelimit)

// PUBLIC DATA DEFINITIONS -------------------------------------------------

CVAR (Bool,	sb_cooperative_enable,				true,		CVAR_ARCHIVE)
CVAR (Int,	sb_cooperative_headingcolor,		CR_RED,		CVAR_ARCHIVE)
CVAR (Int,	sb_cooperative_yourplayercolor,		CR_GREEN,	CVAR_ARCHIVE)
CVAR (Int,	sb_cooperative_otherplayercolor,	CR_GREY,	CVAR_ARCHIVE)

CVAR (Bool,	sb_deathmatch_enable,				true,		CVAR_ARCHIVE)
CVAR (Int,	sb_deathmatch_headingcolor,			CR_RED,		CVAR_ARCHIVE)
CVAR (Int,	sb_deathmatch_yourplayercolor,		CR_GREEN,	CVAR_ARCHIVE)
CVAR (Int,	sb_deathmatch_otherplayercolor,		CR_GREY,	CVAR_ARCHIVE)

CVAR (Bool,	sb_teamdeathmatch_enable,			true,		CVAR_ARCHIVE)
CVAR (Int,	sb_teamdeathmatch_headingcolor,		CR_RED,		CVAR_ARCHIVE)

int comparepoints (const void *arg1, const void *arg2)
{
	// Compare first be frags/kills, then by name.
	player_t *p1 = *(player_t **)arg1;
	player_t *p2 = *(player_t **)arg2;
	int diff;

	diff = deathmatch ? p2->fragcount - p1->fragcount : p2->killcount - p1->killcount;
	if (diff == 0)
	{
		diff = stricmp(p1->userinfo.GetName(), p2->userinfo.GetName());
	}
	return diff;
}

int compareteams (const void *arg1, const void *arg2)
{
	// Compare first by teams, then by frags, then by name.
	player_t *p1 = *(player_t **)arg1;
	player_t *p2 = *(player_t **)arg2;
	int diff;

	diff = p1->userinfo.GetTeam() - p2->userinfo.GetTeam();
	if (diff == 0)
	{
		diff = p2->fragcount - p1->fragcount;
		if (diff == 0)
		{
			diff = stricmp (p1->userinfo.GetName(), p2->userinfo.GetName());
		}
	}
	return diff;
}

/*
void HU_SortPlayers
{
	if (teamplay)
	qsort(sortedplayers, MAXPLAYERS, sizeof(player_t *), compareteams);
	else
		qsort(sortedplayers, MAXPLAYERS, sizeof(player_t *), comparepoints);
}
*/

bool bScoreboardToggled = false;


// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// HU_DrawScores
//
//==========================================================================

void HU_DrawScores(double ticFrac)
{
	bool res = false;
	if (StatusBar != nullptr)
	{
		IFVIRTUALPTR(StatusBar, DBaseStatusBar, DrawScoreboard)
			res = CallVM<int>(func, StatusBar, ticFrac);
	}

	if (!res)
	{
		bScoreboardToggled = false;
	}
}

CCMD(togglescoreboard)
{
	bScoreboardToggled = !bScoreboardToggled;
}
