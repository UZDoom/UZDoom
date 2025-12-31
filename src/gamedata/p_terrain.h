/*
** p_terrain.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __P_TERRAIN_H__
#define __P_TERRAIN_H__

#include "s_sound.h"
#include "textures.h"

class PClass;

extern uint16_t DefaultTerrainType;


class FTerrainTypeArray
{
public:
	TArray<uint16_t> Types;

	uint16_t operator [](FTextureID tex) const
	{
		if ((unsigned)tex.GetIndex() >= Types.Size()) return DefaultTerrainType;
		uint16_t type = Types[tex.GetIndex()];
		return type == 0xffff? DefaultTerrainType : type;
	}
	uint16_t operator [](int texnum) const
	{
		if ((unsigned)texnum >= Types.Size()) return DefaultTerrainType;
		uint16_t type = Types[texnum];
		return type == 0xffff? DefaultTerrainType : type;
	}
	void Resize(unsigned newsize)
	{
		Types.Resize(newsize);
	}
	void Clear()
	{
		memset (&Types[0], 0xff, Types.Size()*sizeof(uint16_t));
	}
	void Set(int index, int value)
	{
		if ((unsigned)index >= Types.Size())
		{
			int oldsize = Types.Size();
			Resize(index + 1);
			memset(&Types[oldsize], 0xff, (index + 1 - oldsize)*sizeof(uint16_t));
		}
		Types[index] = value;
	}
};

extern FTerrainTypeArray TerrainTypes;

// at game start
void P_InitTerrainTypes ();

struct FSplashDef
{
	FName Name;
	FSoundID SmallSplashSound;
	FSoundID NormalSplashSound;
	PClassActor *SmallSplash;
	PClassActor *SplashBase;
	PClassActor *SplashChunk;
	uint8_t ChunkXVelShift;
	uint8_t ChunkYVelShift;
	uint8_t ChunkZVelShift;
	bool NoAlert;
	double ChunkBaseZVel;
	double SmallSplashClip;
};

struct FTerrainDef
{
	FName Name;
	int Splash;
	int DamageAmount;
	FName DamageMOD;
	int DamageTimeMask;
	double FootClip;
	float StepVolume;
	int WalkStepTics;
	int RunStepTics;
	FSoundID LeftStepSound;
	FSoundID RightStepSound;
	bool IsLiquid;
	bool AllowProtection;
	bool DamageOnLand;
	double Friction;
	double MoveFactor;
	FSoundID StepSound;
	double StepDistance;
	double StepDistanceMinVel;
};

extern TArray<FSplashDef> Splashes;
extern TArray<FTerrainDef> Terrains;

int P_FindTerrain(FName name);
FName P_GetTerrainName(int terrainnum);

#endif //__P_TERRAIN_H__
