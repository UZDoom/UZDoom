/*
** p_destructible.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018 ZZYZX
** Copyright 2018-2025 GZDoom Maintainers and Contributors
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
** Code written prior to 2026 is also licensed under:
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

#include "tarray.h"
#include "r_defs.h"
#include "p_trace.h"

// [ZZ] Destructible geometry related
struct FHealthGroup
{
	TArray<sector_t*> sectors;
	TArray<line_t*> lines;
	int health;
	int id;
};

// for P_DamageSector
enum
{
	SECPART_Floor = 0,
	SECPART_Ceiling = 1,
	SECPART_3D = 2
};

struct FLevelLocals;
void P_InitHealthGroups(FLevelLocals *Level);

void P_SetHealthGroupHealth(FHealthGroup* group, int health);
void P_SetHealthGroupHealth(FLevelLocals *Level, int group, int health);

FHealthGroup* P_GetHealthGroup(FLevelLocals *Level, int id);
FHealthGroup* P_GetHealthGroupOrNew(FLevelLocals *Level, int id, int startinghealth);

void P_DamageSector(sector_t* sector, AActor* source, int damage, FName damagetype, int part, DVector3 position, bool isradius, bool dogroups);
void P_DamageLinedef(line_t* line, AActor* source, int damage, FName damagetype, int side, DVector3 position, bool isradius, bool dogroups);

void P_GeometryLineAttack(FTraceResults& trace, AActor* thing, int damage, FName damageType);
void P_GeometryRadiusAttack(AActor* bombspot, AActor* bombsource, int bombdamage, double bombdistance, FName damagetype, double fulldamagedistance);
bool P_ProjectileHitLinedef(AActor* projectile, line_t* line);
bool P_ProjectileHitPlane(AActor* projectile, int part);

bool P_CheckLinedefVulnerable(line_t* line, int side, int part = -1);
bool P_CheckSectorVulnerable(sector_t* sector, int part);

void P_SerializeHealthGroups(FLevelLocals *Level, FSerializer& arc);
