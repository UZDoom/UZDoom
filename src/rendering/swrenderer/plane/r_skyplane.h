/*
** r_skyplane.h
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

#include "r_visibleplane.h"
#include "swrenderer/viewport/r_skydrawer.h"

namespace swrenderer
{
	class RenderSkyPlane
	{
	public:
		RenderSkyPlane(RenderThread *thread);

		void Render(VisiblePlane *pl);

		RenderThread *Thread = nullptr;

	private:
		void DrawSky(VisiblePlane *pl);
		void DrawSkyColumnStripe(int start_x, int y1, int y2, double scale, double texturemid, double yrepeat);
		void DrawSkyColumn(int start_x, int y1, int y2);

		double		skytexturemid;
		double		skyscale;
		float		skyiscale;
		fixed_t		sky1cyl, sky2cyl;

		FSoftwareTexture *frontskytex = nullptr;
		FSoftwareTexture *backskytex = nullptr;
		angle_t skyflip = 0;
		int frontpos = 0;
		int backpos = 0;
		fixed_t frontcyl = 0;
		fixed_t backcyl = 0;
		double skymid = 0.0;
		angle_t skyangle = 0;
		int skyoffset = 0;

		SkyDrawerArgs drawerargs;
	};
}
