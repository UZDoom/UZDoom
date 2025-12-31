/*
** p_enemy.h
**
** Enemy thinking, AI.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
** Copyright 1998-1998 Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
*/

#ifndef __P_ENEMY_H__
#define __P_ENEMY_H__

#include "dobject.h"
#include "vectors.h"

struct sector_t;
class AActor;
class PClass;
struct FState;


enum dirtype_t
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
};

extern double xspeed[8], yspeed[8];

enum LO_Flags
{
	LOF_NOSIGHTCHECK = 1,
	LOF_NOSOUNDCHECK = 2,
	LOF_DONTCHASEGOAL = 4,
	LOF_NOSEESOUND = 8,
	LOF_FULLVOLSEESOUND = 16,
    LOF_NOJUMP = 32,
};

struct FLookExParams
{
	DAngle Fov;
	double minDist;
	double maxDist;
	double maxHeardist;
	int flags;
	FState *seestate;
};

int P_HitFriend (AActor *self);
void P_NoiseAlert (AActor *emitter, AActor *target, bool splash=false, double maxdist=0);
int P_CheckMeleeRange(AActor* actor, double range = -1);

bool P_CheckMeleeRange2 (AActor *actor);
int P_SmartMove (AActor *actor);
bool P_TryWalk (AActor *actor);
void P_NewChaseDir (AActor *actor);
void P_RandomChaseDir(AActor *actor);;
int P_IsVisible(AActor *lookee, AActor *other, INTBOOL allaround, FLookExParams *params);

AActor *P_DropItem (AActor *source, PClassActor *type, int special, int chance);
int P_LookForMonsters(AActor *actor);
int P_LookForTID(AActor *actor, INTBOOL allaround, FLookExParams *params);
int P_LookForEnemies(AActor *actor, INTBOOL allaround, FLookExParams *params);
int P_LookForPlayers (AActor *actor, INTBOOL allaround, FLookExParams *params);
void A_Weave(AActor *self, int xyspeed, int zspeed, double xydist, double zdist);
void A_Unblock(AActor *self, bool drop);

void A_BossDeath(AActor *self);

void A_Wander(AActor *self, int flags = 0);
void A_DoChase(AActor *actor, bool fastchase, FState *meleestate, FState *missilestate, bool playactive, bool nightmarefast, bool dontmove, int flags);
void A_Chase(AActor *self);
void A_FaceTarget(AActor *actor);
void A_Face(AActor *self, AActor *other, DAngle max_turn = nullAngle, DAngle max_pitch = DAngle::fromDeg(270.), DAngle ang_offset = nullAngle, DAngle pitch_offset = nullAngle, int flags = 0, double z_add = 0);
class FSoundID;

int CheckBossDeath (AActor *);

#endif //__P_ENEMY_H__
