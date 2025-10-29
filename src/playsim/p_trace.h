/*
** p_trace.h
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

#ifndef __P_TRACE_H__
#define __P_TRACE_H__

#include <stddef.h>
#include "actor.h"
#include "cmdlib.h"
#include "textures.h"

struct sector_t;
struct line_t;
class AActor;
struct F3DFloor;

enum ETraceResult
{
	TRACE_HitNone,
	TRACE_HitFloor,
	TRACE_HitCeiling,
	TRACE_HitWall,
	TRACE_HitActor,
	TRACE_CrossingPortal,
	TRACE_HasHitSky,
};

enum
{
	TIER_Middle,
	TIER_Upper,
	TIER_Lower,
	TIER_FFloor,
};

struct FTraceResults
{
	sector_t *Sector;
	FTextureID HitTexture;
	DVector3 HitPos;
	DVector3 HitVector;
	DVector3 SrcFromTarget;
	DAngle SrcAngleFromTarget;

	double Distance;
	double Fraction;

	AActor *Actor;		// valid if hit an actor

	line_t *Line;		// valid if hit a line
	uint8_t Side;
	uint8_t Tier;
	bool unlinked;		// passed through a portal without static offset.
	ETraceResult HitType;
	F3DFloor *ffloor;

	sector_t *CrossedWater;		// For Boom-style, Transfer_Heights-based deep water
	DVector3 CrossedWaterPos;	// remember the position so that we can use it for spawning the splash
	F3DFloor *Crossed3DWater;	// For 3D floor-based deep water
	DVector3 Crossed3DWaterPos;
};
	

enum
{
	TRACE_NoSky			= 0x0001,	// Hitting the sky returns TRACE_HitNone
	TRACE_PCross		= 0x0002,	// Trigger SPAC_PCROSS lines
	TRACE_Impact		= 0x0004,	// Trigger SPAC_IMPACT lines
	TRACE_PortalRestrict= 0x0008,	// Cannot go through portals without a static link offset.
	TRACE_ReportPortals = 0x0010,	// Report any portal crossing to the TraceCallback
	TRACE_3DCallback	= 0x0020,	// [ZZ] use TraceCallback to determine whether we need to go through a line to do 3D floor check, or not. without this, only line flag mask is used
	TRACE_HitSky		= 0x0040,	// Hitting the sky returns TRACE_HasHitSky
};

// return values from callback
enum ETraceStatus
{
	TRACE_Stop,			// stop the trace, returning this hit
	TRACE_Continue,		// continue the trace, returning this hit if there are none further along
	TRACE_Skip,			// continue the trace; do not return this hit
	TRACE_Abort,		// stop the trace, returning no hits
	TRACE_ContinueOutOfBounds,	// continue the trace through walls; don't use this for railguns
};

bool Trace(const DVector3 &start, sector_t *sector, const DVector3 &direction, double maxDist,
	ActorFlags ActorMask, uint32_t WallMask, AActor *ignore, FTraceResults &res, uint32_t traceFlags = 0,
	ETraceStatus(*callback)(FTraceResults &res, void *) = NULL, void *callbackdata = NULL);

// [ZZ] this is the object that's used for ZScript
class DLineTracer : public DObject
{
	DECLARE_CLASS(DLineTracer, DObject)
public:
	FTraceResults Results;
	static ETraceStatus TraceCallback(FTraceResults& res, void* pthis);
	ETraceStatus CallZScriptCallback();
};

#endif //__P_TRACE_H__
