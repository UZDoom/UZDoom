/*
** r_portalsegment.cpp
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

#include "doomdef.h"
#include "m_bbox.h"

#include "p_lnspec.h"
#include "p_setup.h"
#include "swrenderer/drawers/r_draw.h"
#include "swrenderer/scene/r_3dfloors.h"
#include "a_sharedglobal.h"
#include "g_level.h"
#include "p_effect.h"
#include "doomstat.h"
#include "r_state.h"
#include "swrenderer/scene/r_opaque_pass.h"
#include "v_palette.h"
#include "r_sky.h"
#include "po_man.h"
#include "r_data/colormaps.h"
#include "swrenderer/segments/r_portalsegment.h"
#include "r_memory.h"
#include "swrenderer/r_renderthread.h"

namespace swrenderer
{
	PortalDrawseg::PortalDrawseg(RenderThread *thread, line_t *linedef, int x1, int x2, const short *topclip, const short *bottomclip) : x1(x1), x2(x2)
	{
		src = linedef;
		dst = linedef->special == Line_Mirror ? linedef : linedef->getPortalDestination();
		len = x2 - x1;

		ceilingclip = thread->FrameMemory->AllocMemory<short>(len);
		floorclip = thread->FrameMemory->AllocMemory<short>(len);
		memcpy(ceilingclip, topclip + x1, len * sizeof(short));
		memcpy(floorclip, bottomclip + x1, len * sizeof(short));

		mirror = linedef->special == Line_Mirror;
	}
}
