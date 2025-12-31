/*
** p_linetracedata.h
**
** Structure for passing detailed results of LineTrace to ZScript
**
**---------------------------------------------------------------------------
**
** Copyright 2018-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef P_LTRACEDATA_H
#define P_LTRACEDATA_H

struct FLineTraceData
{
	AActor *HitActor;
	line_t *HitLine;
	sector_t *HitSector;
	F3DFloor *Hit3DFloor;
	FTextureID HitTexture;
	DVector3 HitLocation;
	DVector3 HitDir;
	double Distance;
	int NumPortals;
	int LineSide;
	int LinePart;
	int SectorPlane;
	ETraceResult HitType;
};

int P_LineTrace(AActor *t1, DAngle angle, double distance,
				 DAngle pitch, int flags, double sz, double offsetforward,
				 double offsetside, FLineTraceData *outdata);

#endif
