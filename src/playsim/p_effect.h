/*
** p_effect.h
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

#pragma once

#include "vectors.h"
#include "doomdef.h"
#include "renderstyle.h"
#include "dthinker.h"
#include "palettecontainer.h"
#include "animations.h"

enum
{
	FX_ROCKET			= 0x00000001,
	FX_GRENADE			= 0x00000002,
	FX_RESPAWNINVUL		= 0x00000020,
	FX_VISIBILITYPULSE	= 0x00000040
};

struct subsector_t;
struct FLevelLocals;

// [RH] Particle details

enum EParticleStyle
{
	PT_DEFAULT	= -1, // Use gl_particles_style
	PT_SQUARE	= 0,
	PT_ROUND	= 1,
	PT_SMOOTH	= 2,
};

enum EParticleFlags
{
	SPF_FULLBRIGHT				= 1 << 0,
	SPF_RELPOS					= 1 << 1,
	SPF_RELVEL					= 1 << 2,
	SPF_RELACCEL				= 1 << 3,
	SPF_RELANG					= 1 << 4,
	SPF_NOTIMEFREEZE			= 1 << 5,
	SPF_ROLL					= 1 << 6,
	SPF_REPLACE					= 1 << 7,
	SPF_NO_XY_BILLBOARD			= 1 << 8,
	SPF_LOCAL_ANIM				= 1 << 9,
	SPF_NEGATIVE_FADESTEP		= 1 << 10,
	SPF_FACECAMERA				= 1 << 11,
	SPF_NOFACECAMERA			= 1 << 12,
	SPF_ROLLCENTER				= 1 << 13,
	SPF_STRETCHPIXELS			= 1 << 14,
	SPF_ALLOWSHADERS			= 1 << 15,
	SPF_FADE_IN_OUT				= 1 << 16,
	SPF_FADE_IN_HOLD_OUT		= 1 << 17,
	SPF_NODYNAMICLIGHTING		= 1 << 18,
};

class DVisualThinker;
struct particle_t
{
	subsector_t* subsector; //+8 = 8
    DVector3 Pos; //+24 = 32
    FVector3 Vel; //+12 = 44
    FVector3 Acc; //+12 = 56
	float    size, sizestep; //+8 = 64
    float    fadestep, alpha; //+8 = 72
    int32_t    ttl; // +4 = 76
    int        color; //+4 = 80
    FTextureID texture; // +4 = 84
    ERenderStyle style; //+4 = 88
    float Roll, RollVel, RollAcc; //+12 = 100
    uint16_t    tnext, snext, tprev; //+6 = 106
	// uint16_t padding; //+2 = 108
	uint32_t flags; //+4 = 112
	FStandaloneAnimation animData; //+16 = 128
	float	fadeoutstep; //+4 = 132
	// float padding2; //+4 = 136
};

static_assert(sizeof(particle_t) == 136, "Only LP64/LLP64 is supported");

const uint16_t NO_PARTICLE = 0xffff;

void P_InitParticles(FLevelLocals *);
void P_ClearParticles (FLevelLocals *Level);
void P_FindParticleSubsectors (FLevelLocals *Level);


class AActor;

particle_t *JitterParticle (FLevelLocals *Level, int ttl);
particle_t *JitterParticle (FLevelLocals *Level, int ttl, double drift);

void P_ThinkParticles (FLevelLocals *Level);

struct FSpawnParticleParams
{
	int color;
	FTextureID texture;
	int style;
	int flags;
	int lifetime;

	double size;
	double sizestep;

	DVector3 pos;
	DVector3 vel;
	DVector3 accel;

	double startalpha;
	double fadestep;
	double fadeoutstep;

	double startroll;
	double rollvel;
	double rollacc;
};

void P_SpawnParticle(FLevelLocals *Level, const DVector3 &pos, const DVector3 &vel, const DVector3 &accel, PalEntry color, double startalpha, int lifetime, double size, double fadestep, double sizestep, int flags = 0, FTextureID texture = FNullTextureID(), ERenderStyle style = STYLE_None, double startroll = 0, double rollvel = 0, double rollacc = 0, double fadeoutstep = 0);

void P_InitEffects (void);

void P_RunEffect (AActor *actor, int effects);

struct SPortalHit
{
	DVector3 HitPos;
	DVector3 ContPos;
	DVector3 OutDir;
};

void P_DrawRailTrail(AActor *source, TArray<SPortalHit> &portalhits, int color1, int color2, double maxdiff = 0, int flags = 0, PClassActor *spawnclass = NULL, DAngle angle = nullAngle, int duration = TICRATE, double sparsity = 1.0, double drift = 1.0, int SpiralOffset = 270, DAngle pitch = nullAngle);
void P_DrawSplash (FLevelLocals *Level, int count, const DVector3 &pos, DAngle angle, int kind);
void P_DrawSplash2 (FLevelLocals *Level, int count, const DVector3 &pos, DAngle angle, int updown, int kind);
void P_DisconnectEffect (AActor *actor);
