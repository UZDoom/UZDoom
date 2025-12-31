/*
** r_walldrawer.cpp
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

#include <stddef.h>
#include "r_walldrawer.h"
#include "swrenderer/r_renderthread.h"

namespace swrenderer
{
	void WallDrawerArgs::SetDest(RenderViewport *viewport)
	{
		dc_viewport = viewport;
	}

	void WallDrawerArgs::DrawWall(RenderThread *thread)
	{
		(thread->Drawers(dc_viewport)->*wallfunc)(*this);
	}

	void WallDrawerArgs::SetStyle(bool masked, bool additive, fixed_t alpha, bool dynlights)
	{
		if (alpha < OPAQUE || additive)
		{
			if (!additive && !dynlights)
			{
				wallfunc = &SWPixelFormatDrawers::DrawWallAdd;
				dc_srcblend = Col2RGB8[alpha >> 10];
				dc_destblend = Col2RGB8[(OPAQUE - alpha) >> 10];
				dc_srcalpha = alpha;
				dc_destalpha = OPAQUE - alpha;
			}
			else if (!additive)
			{
				wallfunc = &SWPixelFormatDrawers::DrawWallAddClamp;
				dc_srcblend = Col2RGB8_LessPrecision[alpha >> 10];
				dc_destblend = Col2RGB8_LessPrecision[(OPAQUE - alpha) >> 10];
				dc_srcalpha = alpha;
				dc_destalpha = OPAQUE - alpha;
			}
			else
			{
				wallfunc = &SWPixelFormatDrawers::DrawWallAddClamp;
				dc_srcblend = Col2RGB8_LessPrecision[alpha >> 10];
				dc_destblend = Col2RGB8_LessPrecision[FRACUNIT >> 10];
				dc_srcalpha = alpha;
				dc_destalpha = FRACUNIT;
			}
		}
		else if (masked)
		{
			wallfunc = &SWPixelFormatDrawers::DrawWallMasked;
		}
		else
		{
			wallfunc = &SWPixelFormatDrawers::DrawWall;
		}
	}
}
