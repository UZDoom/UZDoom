/*
** r_visibleplanelist.h
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

#include <stddef.h>
#include "r_defs.h"

struct FSectorPortal;

namespace swrenderer
{
	class RenderThread;
	struct VisiblePlane;

	class VisiblePlaneList
	{
	public:
		VisiblePlaneList(RenderThread *thread);

		void Clear();
		void ClearKeepFakePlanes();

		VisiblePlane *FindPlane(const secplane_t &height, FTextureID picnum, int lightlevel, bool foggy, double Alpha, bool additive, const FTransform &xxform, int sky, FSectorPortal *portal, FDynamicColormap *basecolormap, Fake3DOpaque::Type fakeFloorType, fixed_t fakeAlpha);
		VisiblePlane *GetRange(VisiblePlane *pl, int start, int stop);

		bool HasPortalPlanes() const;
		VisiblePlane *PopFirstPortalPlane();
		void ClearPortalPlanes();

		int Render();
		void RenderHeight(double height);

		RenderThread *Thread = nullptr;

	private:
		VisiblePlaneList();
		VisiblePlane *Add(unsigned hash);

		enum { MAXVISPLANES = 128 }; // must be a power of 2
		VisiblePlane *visplanes[MAXVISPLANES + 1];

		static unsigned CalcHash(int picnum, int lightlevel, const secplane_t &height) { return (unsigned)((picnum) * 3 + (lightlevel)+(FLOAT2FIXED((height).fD())) * 7) & (MAXVISPLANES - 1); }
	};
}
