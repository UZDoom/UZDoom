/*
** r_farclip_line.h
**
**
**
**---------------------------------------------------------------------------
**
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

#include "r_line.h"

namespace swrenderer
{
	class FarClipLine : VisibleSegmentRenderer
	{
	public:
		FarClipLine(RenderThread *thread);
		void Render(seg_t *line, subsector_t *subsector, VisiblePlane *linefloorplane, VisiblePlane *lineceilingplane, Fake3DOpaque opaque3dfloor);

		RenderThread *Thread = nullptr;

	private:
		bool RenderWallSegment(int x1, int x2) override;

		void ClipSegmentTopBottom(int x1, int x2);
		void MarkCeilingPlane(int x1, int x2);
		void MarkFloorPlane(int x1, int x2);

		subsector_t *mSubsector;
		sector_t *mFrontSector;
		seg_t *mLineSegment;
		VisiblePlane *mFloorPlane;
		VisiblePlane *mCeilingPlane;
		Fake3DOpaque m3DFloor;

		double mFrontCeilingZ1;
		double mFrontCeilingZ2;
		double mFrontFloorZ1;
		double mFrontFloorZ2;

		FWallCoords WallC;

		bool mPrepped;

		ProjectedWallLine walltop;
		ProjectedWallLine wallbottom;
	};
}
