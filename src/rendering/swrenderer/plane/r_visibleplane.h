/*
** r_visibleplane.h
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
#include "r_state.h"
#include "r_memory.h"

struct FDynamicLight;
struct FLightNode;
struct FDynamicColormap;
struct FSectorPortal;

namespace swrenderer
{
	class RenderThread;

	struct VisiblePlaneLight
	{
		FDynamicLight *lightsource;
		VisiblePlaneLight *next;
	};

	struct VisiblePlane
	{
		VisiblePlane(RenderThread *thread);

		void AddLights(RenderThread *thread, FSection *sec);
		void Render(RenderThread *thread, fixed_t alpha, bool additive, bool masked);

		VisiblePlane *next = nullptr;		// Next visplane in hash chain -- killough

		FDynamicColormap *colormap = nullptr;		// [RH] Support multiple colormaps
		FSectorPortal *portal = nullptr;			// [RH] Support sky boxes
		VisiblePlaneLight *lights = nullptr;

		FTransform	xform;
		secplane_t	height;
		FTextureID	picnum;
		int lightlevel = 0;
		int left = viewwidth;
		int right = 0;
		int sky = 0;

		// [RH] This set of variables copies information from the time when the
		// visplane is created. They are only used by stacks so that you can
		// have stacked sectors inside a skybox. If the visplane is not for a
		// stack, then they are unused.
		int			extralight = 0;
		double		visibility = 0.0;
		DVector3	viewpos = { 0.0, 0.0, 0.0 };
		DAngle		viewangle = nullAngle;
		fixed_t		Alpha = 0;
		bool		Additive = false;

		// kg3D - keep track of mirror and skybox owner
		int CurrentSkybox = 0;
		int CurrentPortalUniq = 0; // mirror counter, counts all of them
		int MirrorFlags = 0; // this is not related to CurrentMirror

		uint16_t *bottom = nullptr;
		uint16_t *top = nullptr;
	};
}
