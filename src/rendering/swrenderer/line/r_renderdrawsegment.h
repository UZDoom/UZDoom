/*
** r_renderdrawsegment.h
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

#include "swrenderer/segments/r_drawsegment.h"

namespace swrenderer
{
	class RenderThread;

	class RenderDrawSegment
	{
	public:
		RenderDrawSegment(RenderThread *thread);
		void Render(DrawSegment *ds, int x1, int x2, Fake3DTranslucent clip3DFloor);

		RenderThread *Thread = nullptr;

	private:
		void RenderWall(DrawSegment *ds, int x1, int x2);
		void Render3DFloorWall(DrawSegment *ds, int x1, int x2, F3DFloor *rover, double clipTop, double clipBottom, FSoftwareTexture *rw_pic);
		void Render3DFloorWallRange(DrawSegment *ds, int x1, int x2);
		void RenderFog(DrawSegment* ds, int x1, int x2);
		void ClipMidtex(DrawSegment* ds, int x1, int x2);
		void GetNoWrapMidTextureZ(DrawSegment* ds, FSoftwareTexture* tex, double& ceilZ, double& floorZ);
		void GetMaskedWallTopBottom(DrawSegment *ds, double &top, double &bot);

		sector_t *frontsector = nullptr;
		sector_t *backsector = nullptr;

		seg_t *curline = nullptr;
		Fake3DTranslucent m3DFloor;

		ProjectedWallLine wallupper;
		ProjectedWallLine walllower;
	};
}
