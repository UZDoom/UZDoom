/*
** r_visibleplane.cpp
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
#include "a_dynlight.h"
#include "texturemanager.h"
#include "r_memory.h"
#include "swrenderer/r_renderthread.h"
#include "swrenderer/scene/r_opaque_pass.h"
#include "swrenderer/scene/r_3dfloors.h"
#include "swrenderer/scene/r_portal.h"
#include "swrenderer/scene/r_light.h"
#include "swrenderer/plane/r_flatplane.h"
#include "swrenderer/plane/r_slopeplane.h"
#include "swrenderer/plane/r_skyplane.h"
#include "swrenderer/plane/r_visibleplane.h"
#include "swrenderer/drawers/r_draw.h"

CVAR(Bool, tilt, false, 0);

namespace swrenderer
{
	VisiblePlane::VisiblePlane(RenderThread *thread)
	{
		picnum.SetNull();
		height.set(0.0, 0.0, 1.0, 0.0);

		bottom = thread->FrameMemory->AllocMemory<uint16_t>(viewwidth);
		top = thread->FrameMemory->AllocMemory<uint16_t>(viewwidth);

		fillshort(bottom, viewwidth, 0);
		fillshort(top, viewwidth, 0x7fff);
	}

	void VisiblePlane::AddLights(RenderThread *thread, FSection *sec)
	{
		if (!r_dynlights)
			return;

		CameraLight *cameraLight = CameraLight::Instance();
		if (cameraLight->FixedColormap() != NULL || cameraLight->FixedLightLevel() >= 0)
			return; // [SP] no dynlights if invul or lightamp

		auto Level = sec->sector->Level;

		if (Level->lightlists.flat_dlist.SSize() > sec->Index())
		{
			TMap<FDynamicLight *, std::unique_ptr<FLightNode>>::Iterator it(Level->lightlists.flat_dlist[sec->Index()]);
			TMap<FDynamicLight *, std::unique_ptr<FLightNode>>::Pair *pair;
			while (it.NextPair(pair))
			{
				auto node = pair->Value.get();
				if (!node) continue;

				if (node->lightsource->IsActive() && (height.PointOnSide(node->lightsource->Pos) > 0))
				{
					bool found = false;
					VisiblePlaneLight *light_node = lights;
					while (light_node)
					{
						if (light_node->lightsource == node->lightsource)
						{
							found = true;
							break;
						}
						light_node = light_node->next;
					}
					if (!found)
					{
						VisiblePlaneLight *newlight = thread->FrameMemory->NewObject<VisiblePlaneLight>();
						newlight->next = lights;
						newlight->lightsource = node->lightsource;
						lights = newlight;
					}
				}
			}
		}
	}

	void VisiblePlane::Render(RenderThread *thread, fixed_t alpha, bool additive, bool masked)
	{
		if (left >= right)
			return;

		if (picnum == skyflatnum) // sky flat
		{
			RenderSkyPlane renderer(thread);
			renderer.Render(this);
		}
		else // regular flat
		{
			auto tex = GetPalettedSWTexture(picnum, true);
			if (tex == nullptr)
				return;
			if (!masked && !additive)
			{ // If we're not supposed to see through this plane, draw it opaque.
				alpha = OPAQUE;
			}
			else if (!tex->isMasked())
			{ // Don't waste time on a masked texture if it isn't really masked.
				masked = false;
			}
			double xscale = xform.xScale * tex->GetScale().X;
			double yscale = xform.yScale * tex->GetScale().Y;

			if (!height.isSlope() && !tilt)
			{
				RenderFlatPlane renderer(thread);
				renderer.Render(this, xscale, yscale, alpha, additive, masked, colormap, tex);
			}
			else
			{
				RenderSlopePlane renderer(thread);
				renderer.Render(this, xscale, yscale, alpha, additive, masked, colormap, tex);
			}
		}
	}
}
