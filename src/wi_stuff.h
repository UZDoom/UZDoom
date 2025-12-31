/*
** wi_stuff.h
**
** Intermission.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef __WI_STUFF__
#define __WI_STUFF__

#include "doomdef.h"

struct FLevelLocals;

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
struct wbplayerstruct_t
{
	// Player stats, kills, collected items etc.
	int			skills;
	int			sitems;
	int			ssecret;
	int			stime;
	int			frags[MAXPLAYERS];
	int			fragcount;	// [RH] Cumulative frags for this player
};

struct wbstartstruct_t
{
	int			finished_ep;
	int			next_ep;

	FString		current;	// [RH] Name of map just finished
	FString		next;		// next level, [RH] actual map name
	FString		nextname;	// printable name for next level.
	FString		thisname;	// printable name for next level.
	FString		nextauthor;	// printable name for next level.
	FString		thisauthor;	// printable name for next level.

	FTextureID	LName0;
	FTextureID	LName1;

	int			totalkills;
	int			maxkills;
	int			maxitems;
	int			maxsecret;
	int			maxfrags;

	// the par time and sucktime
	int			partime;	// in tics
	int			sucktime;	// in minutes

	// total time for the entire current game
	int			totaltime;

	// index of this player in game
	int			pnum;

	wbplayerstruct_t	plyr[MAXPLAYERS];
	
};

// Setup for an intermission screen.
DObject* WI_Start (wbstartstruct_t *wbstartstruct);

#endif
