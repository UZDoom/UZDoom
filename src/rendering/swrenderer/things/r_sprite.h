/*
** r_sprite.h
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

#include "r_visiblesprite.h"

namespace swrenderer
{
	class RenderSprite : public VisibleSprite
	{
	public:
		static void Project(RenderThread *thread, AActor *thing, const DVector3 &pos, FSoftwareTexture *tex, const DVector2 &spriteScale, int renderflags, WaterFakeSide fakeside, F3DFloor *fakefloor, F3DFloor *fakeceiling, sector_t *current_sector, int lightlevel, bool foggy, FDynamicColormap *basecolormap, bool isSpriteShadow = false);

	protected:
		void Render(RenderThread *thread, short *cliptop, short *clipbottom, int minZ, int maxZ, Fake3DTranslucent clip3DFloor) override;

	private:
		FWallCoords wallc;
		double SpriteScale;

		FTranslationID Translation = NO_TRANSLATION;
		uint32_t FillColor = 0;
		
		uint32_t dynlightcolor = 0;
	};
}
