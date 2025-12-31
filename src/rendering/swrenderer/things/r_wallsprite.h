/*
** r_wallsprite.h
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
	class ProjectedWallTexcoords;
	class SpriteDrawerArgs;

	class RenderWallSprite : public VisibleSprite
	{
	public:
		static void Project(RenderThread *thread, AActor *thing, const DVector3 &pos, FSoftwareTexture *pic, const DVector2 &scale, int renderflags, int lightlevel, bool foggy, FDynamicColormap *basecolormap);

	protected:
		bool IsWallSprite() const override { return true; }
		void Render(RenderThread *thread, short *cliptop, short *clipbottom, int minZ, int maxZ, Fake3DTranslucent clip3DFloor) override;

	private:
		FWallCoords wallc;
		FTranslationID Translation = NO_TRANSLATION;
		uint32_t FillColor = 0;
	};
}
