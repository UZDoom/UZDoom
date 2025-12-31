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
