/*
** r_line.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2016 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "vectors.h"
#include "r_wallsetup.h"
#include "swrenderer/segments/r_clipsegment.h"
#include "swrenderer/scene/r_3dfloors.h"

struct seg_t;
struct subsector_t;
struct sector_t;
struct side_t;
struct line_t;
struct FDynamicColormap;

namespace swrenderer
{
	class RenderThread;
	struct VisiblePlane;

	class SWRenderLine : VisibleSegmentRenderer
	{
	public:
		SWRenderLine(RenderThread *thread);
		void Render(seg_t *line, subsector_t *subsector, sector_t *sector, sector_t *fakebacksector, VisiblePlane *floorplane, VisiblePlane *ceilingplane, Fake3DOpaque fake3DOpaque);

		RenderThread *Thread = nullptr;

	private:
		bool RenderWallSegment(int x1, int x2) override;
		void SetWallVariables();
		void SetTextures();
		void SetTopTexture();
		void SetMiddleTexture();
		void SetBottomTexture();
		void ClipSegmentTopBottom(int x1, int x2);
		void MarkCeilingPlane(int x1, int x2);
		void MarkFloorPlane(int x1, int x2);
		void Mark3DFloors(int x1, int x2);
		void MarkOpaquePassClip(int x1, int x2);
		void RenderTopTexture(int x1, int x2);
		void RenderMiddleTexture(int x1, int x2);
		void RenderBottomTexture(int x1, int x2);

		bool IsFogBoundary(sector_t *front, sector_t *back) const;
		bool SkyboxCompare(sector_t *frontsector, sector_t *backsector) const;

		bool IsInvisibleLine() const;
		bool IsDoorClosed() const;
		bool IsSolid() const;

		bool ShouldMarkFloor() const;
		bool ShouldMarkCeiling() const;
		bool ShouldMarkPortal() const;

		// Line variables:

		subsector_t *mSubsector;
		sector_t *mFrontSector;
		sector_t *mBackSector;
		VisiblePlane *mFloorPlane;
		VisiblePlane *mCeilingPlane;
		seg_t *mLineSegment;
		Fake3DOpaque m3DFloor;

		double mBackCeilingZ1;
		double mBackCeilingZ2;
		double mBackFloorZ1;
		double mBackFloorZ2;
		double mFrontCeilingZ1;
		double mFrontCeilingZ2;
		double mFrontFloorZ1;
		double mFrontFloorZ2;

		bool mDoorClosed;

		FWallCoords WallC;

		// Wall segment variables:

		bool rw_prepped;

		bool markfloor; // False if the back side is the same plane.
		bool markceiling;
		
		FSoftwareTexture* mTopTexture;
		FSoftwareTexture* mMiddleTexture;
		FSoftwareTexture* mBottomTexture;

		ProjectedWallCull mCeilingClipped;
		ProjectedWallCull mFloorClipped;

		ProjectedWallLine walltop;
		ProjectedWallLine wallbottom;
		ProjectedWallLine wallupper;
		ProjectedWallLine walllower;

		sector_t tempsec; // killough 3/8/98: ceiling/water hack
	};
}
