/*
** g_game.h
**
** Duh.
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
** An optional, multi-line description, covering whatever else you need to
** describe in order for people to contribute to this file
*/

#ifndef __G_GAME__
#define __G_GAME__

struct event_t;

#include <functional>

#include "dobjgc.h"
#include "name.h"
#include "gamestate.h"
#include "a_pickups.h"


// wipegamestate can be set to -1
//	to force a wipe on the next draw
extern gamestate_t wipegamestate;



class AActor;
struct FLevelLocals;

//
// GAME
//
enum
{
	PPS_FORCERANDOM			= 1,
	PPS_NOBLOCKINGCHECK		= 2,
};

void G_DeferedPlayDemo (const char* demo);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame (const char* name, bool hidecon=false);

void G_DoLoadGame (void);

// Called by M_Responder.
void G_SaveGame (const char *filename, const char *description, bool quick = false);
// Called by messagebox
void G_DoQuickSave ();

// Only called by startup code.
void G_RecordDemo (const char* name);

void G_BeginRecording (const char *startmap);

void G_PlayDemo (char* name);
void G_TimeDemo (const char* name);
bool G_CheckDemoStatus (void);

void G_Ticker (void);
bool G_Responder (event_t*	ev);

enum
{
	FSTATE_EndingGame = 0,
	FSTATE_ChangingLevel = 1,
	FSTATE_InLevel = 2,
	FSTATE_InLevelNoWipe = 3
};

void G_ScreenShot (const char* filename);
void G_StartSlideshow(FLevelLocals *Level, FName whichone, int state);

class FSerializer;
bool G_CheckSaveGameWads (FSerializer &arc, bool printwarn);

enum EFinishLevelType
{
	FINISH_SameHub,
	FINISH_NextHub,
	FINISH_NoHub
};

void G_PlayerFinishLevel (int player, EFinishLevelType mode, int flags);

void G_DoPlayerPop(int playernum);

// Adds pitch to consoleplayer's viewpitch and clamps it
void G_AddViewPitch (int look, bool mouse = false);

// Adds to consoleplayer's viewangle if allowed
void G_AddViewAngle (int yaw, bool mouse = false);

class FBaseCVar;
FBaseCVar* G_GetUserCVar(int playernum, const char* cvarname);

class DIntermissionController;
struct level_info_t;
void RunIntermission(level_info_t* oldlevel, level_info_t* newlevel, DIntermissionController* intermissionScreen, DObject* statusScreen, bool ending, std::function<void(bool)> completionf);

enum EWeaponSelectType
{
	WST_NONE = NUM_WEAPON_SLOTS,
	WST_PREV,
	WST_NEXT,
};

inline int SendWeaponSlot = WST_NONE;
inline bool WantsFlechetteItem = false;
extern const AActor *SendItemUse, *SendItemDrop;
extern int SendItemDropAmount;


#endif
