/*
** g_level.h
**
** controls movement between levels
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#include "filesystem.h"
#include "g_mapinfo.h"

using FileSys::FCompressedBuffer;
extern bool savegamerestore;

void G_InitNew (const char *mapname, bool bTitleLevel);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferedInitNew (const char *mapname, int skill = -1);
struct FNewGameStartup;
void G_DeferedInitNew (FNewGameStartup *gs);

bool CreateCutscene(struct CutsceneDef* cs, DObject* runner, level_info_t* map);

enum 
{
	CHANGELEVEL_KEEPFACING = 1,
	CHANGELEVEL_RESETINVENTORY = 2,
	CHANGELEVEL_NOMONSTERS = 4,
	CHANGELEVEL_CHANGESKILL = 8,
	CHANGELEVEL_NOINTERMISSION = 16,
	CHANGELEVEL_RESETHEALTH = 32,
	CHANGELEVEL_PRERAISEWEAPON = 64,
};

void G_DoLoadLevel (const FString &MapName, int position, bool autosave, bool newGame);

void G_ClearSnapshots (void);
void P_RemoveDefereds ();
void G_ReadSnapshots (FResourceFile *);
void G_WriteSnapshots (TArray<FString> &, TArray<FCompressedBuffer> &);
void G_WriteVisited(FSerializer &arc);
void G_ReadVisited(FSerializer &arc);
void G_ClearHubInfo();
