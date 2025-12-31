/*
** r_planerenderer.h
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

namespace swrenderer
{
	struct VisiblePlane;

	class PlaneRenderer
	{
	public:
		void RenderLines(VisiblePlane *pl);

		virtual void RenderLine(int y, int x1, int x2) = 0;

	private:
		short spanend[MAXHEIGHT];
	};
}
