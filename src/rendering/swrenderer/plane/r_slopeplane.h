/*
** r_slopeplane.h
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

#include "r_planerenderer.h"
#include "swrenderer/viewport/r_spandrawer.h"

namespace swrenderer
{
	class RenderThread;

	class RenderSlopePlane : PlaneRenderer
	{
	public:
		RenderSlopePlane(RenderThread *thread);
		void Render(VisiblePlane *pl, double _xscale, double _yscale, fixed_t alpha, bool additive, bool masked, FDynamicColormap *basecolormap, FSoftwareTexture *texture);

		RenderThread *Thread = nullptr;

	private:
		void RenderLine(int y, int x1, int x2) override;

		FVector3 plane_sz, plane_su, plane_sv;
		float planelightfloat;
		bool plane_shade;
		int lightlevel;
		bool foggy;
		fixed_t pviewx, pviewy;
		fixed_t xscale, yscale;
		FDynamicColormap *basecolormap;
		SpanDrawerArgs drawerargs;

		DVector3 planeNormal;
		double planeD;
		VisiblePlaneLight* light_list;
	};
}
