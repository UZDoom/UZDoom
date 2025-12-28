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
** Copyright 2025 UZDoom Maintainers and Contributors
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
