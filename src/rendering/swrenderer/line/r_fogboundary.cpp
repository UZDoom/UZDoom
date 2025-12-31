/*
** r_fogboundary.cpp
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

#include <stdlib.h>
#include <float.h>


#include "filesystem.h"
#include "doomdef.h"
#include "doomstat.h"
#include "r_sky.h"
#include "stats.h"
#include "v_video.h"
#include "a_sharedglobal.h"
#include "c_console.h"
#include "cmdlib.h"
#include "d_net.h"
#include "g_level.h"
#include "v_palette.h"
#include "r_data/colormaps.h"
#include "a_dynlight.h"
#include "swrenderer/drawers/r_draw_rgba.h"
#include "swrenderer/scene/r_opaque_pass.h"
#include "swrenderer/scene/r_3dfloors.h"
#include "swrenderer/scene/r_portal.h"
#include "swrenderer/segments/r_clipsegment.h"
#include "swrenderer/segments/r_drawsegment.h"
#include "swrenderer/line/r_fogboundary.h"
#include "r_memory.h"
#include "swrenderer/scene/r_light.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

namespace swrenderer
{
	void RenderFogBoundary::Render(RenderThread *thread, int x1, int x2, const short* uclip, const short* dclip, const ProjectedWallLight &wallLight)
	{
		// This is essentially the same as R_MapVisPlane but with an extra step
		// to create new horizontal spans whenever the light changes enough that
		// we need to use a new colormap.

		int wallshade = LightVisibility::LightLevelToShade(wallLight.GetLightLevel(), wallLight.GetFoggy(), thread->Viewport.get());
		int x = x2 - 1;
		int t2 = uclip[x];
		int b2 = dclip[x];
		float light = wallLight.GetLightPos(x);
		int rcolormap = GETPALOOKUP(light, wallshade);
		int lcolormap;
		FDynamicColormap *basecolormap = wallLight.GetBaseColormap();
		uint8_t *basecolormapdata = basecolormap->Maps;

		if (b2 > t2)
		{
			fillshort(spanend + t2, b2 - t2, x);
		}

		drawerargs.SetBaseColormap(basecolormap);
		drawerargs.SetLight(light, wallLight.GetLightLevel(), wallLight.GetFoggy(), thread->Viewport.get());

		uint8_t *fake_dc_colormap = basecolormap->Maps + (GETPALOOKUP(light, wallshade) << COLORMAPSHIFT);

		for (--x; x >= x1; --x)
		{
			int t1 = uclip[x];
			int b1 = dclip[x];
			const int xr = x + 1;
			int stop;

			light -= wallLight.GetLightStep();
			lcolormap = GETPALOOKUP(light, wallshade);
			if (lcolormap != rcolormap)
			{
				if (t2 < b2 && rcolormap != 0)
				{ // Colormap 0 is always the identity map, so rendering it is
				  // just a waste of time.
					RenderSection(thread, t2, b2, xr);
				}
				if (t1 < t2) t2 = t1;
				if (b1 > b2) b2 = b1;
				if (t2 < b2)
				{
					fillshort(spanend + t2, b2 - t2, x);
				}
				rcolormap = lcolormap;
				drawerargs.SetLight(light, wallshade);
				fake_dc_colormap = basecolormap->Maps + (GETPALOOKUP(light, wallshade) << COLORMAPSHIFT);
			}
			else
			{
				if (fake_dc_colormap != basecolormapdata)
				{
					stop = min(t1, b2);
					while (t2 < stop)
					{
						int y = t2++;
						drawerargs.DrawFogBoundaryLine(thread, y, xr, spanend[y]);
					}
					stop = max(b1, t2);
					while (b2 > stop)
					{
						int y = --b2;
						drawerargs.DrawFogBoundaryLine(thread, y, xr, spanend[y]);
					}
				}
				else
				{
					t2 = max(t2, min(t1, b2));
					b2 = min(b2, max(b1, t2));
				}

				stop = min(t2, b1);
				while (t1 < stop)
				{
					spanend[t1++] = x;
				}
				stop = max(b2, t2);
				while (b1 > stop)
				{
					spanend[--b1] = x;
				}
			}

			t2 = uclip[x];
			b2 = dclip[x];
		}
		if (t2 < b2 && rcolormap != 0)
		{
			RenderSection(thread, t2, b2, x1);
		}
	}

	void RenderFogBoundary::RenderSection(RenderThread *thread, int y, int y2, int x1)
	{
		for (; y < y2; ++y)
		{
			drawerargs.DrawFogBoundaryLine(thread, y, x1, spanend[y]);
		}
	}
}
