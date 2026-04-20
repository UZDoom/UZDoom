/*
** r_portalsegment.h
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

namespace swrenderer
{
	class RenderThread;
	
	/* portal structure, this is used in r_ code in order to store drawsegs with portals (and mirrors) */
	struct PortalDrawseg
	{
		PortalDrawseg(RenderThread *thread, line_t *linedef, int x1, int x2, const short *topclip, const short *bottomclip);

		line_t* src = nullptr; // source line (the one drawn) this doesn't change over render loops
		line_t* dst = nullptr; // destination line (the one that the portal is linked with, equals 'src' for mirrors)

		int x1 = 0; // drawseg x1
		int x2 = 0; // drawseg x2

		int len = 0;
		short *ceilingclip = nullptr;
		short *floorclip = nullptr;

		bool mirror = false; // true if this is a mirror (src should equal dst)
	};
}
