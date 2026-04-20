/*
** r_fogboundary.h
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

#include "swrenderer/viewport/r_spandrawer.h"

namespace swrenderer
{
	class RenderThread;
	
	class RenderFogBoundary
	{
	public:
		void Render(RenderThread *thread, int x1, int x2, const short* uclip, const short* dclip, const ProjectedWallLight &wallLight);

	private:
		void RenderSection(RenderThread *thread, int y, int y2, int x1);

		short spanend[MAXHEIGHT];
		SpanDrawerArgs drawerargs;
	};
}
