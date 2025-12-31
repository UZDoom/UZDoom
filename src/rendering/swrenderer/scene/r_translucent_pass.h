/*
** r_translucent_pass.h
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

#include "tarray.h"

struct particle_t;
struct FVoxel;

namespace swrenderer
{
	class RenderThread;
	class VisibleSprite;
	struct DrawSegment;

	class RenderTranslucentPass
	{
	public:
		RenderTranslucentPass(RenderThread *thread);

		void Deinit();
		void Clear();
		void Render();

		bool ClipSpriteColumnWithPortals(int x, VisibleSprite *spr);

		RenderThread *Thread = nullptr;

	private:
		void CollectPortals();
		void DrawMaskedSingle(bool renew, Fake3DTranslucent clip3DFloor);

		TArray<DrawSegment *> portaldrawsegs;
	};
}
