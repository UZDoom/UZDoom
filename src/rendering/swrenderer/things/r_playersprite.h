/*
** r_playersprite.h
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
#include "r_data/colormaps.h"
#include "palettecontainer.h"

class DPSprite;

namespace swrenderer
{
	class NoAccelPlayerSprite
	{
	public:
		short x1 = 0;
		short x2 = 0;

		double texturemid = 0.0;

		fixed_t xscale = 0;
		float yscale = 0.0f;

		FSoftwareTexture *pic = nullptr;

		fixed_t xiscale = 0;
		fixed_t startfrac = 0;

		float Alpha = 0.0f;
		FRenderStyle RenderStyle;
		FTranslationID Translation = NO_TRANSLATION;
		uint32_t FillColor = 0;

		ColormapLight Light;

		short renderflags = 0;

		void Render(RenderThread *thread);
	};

	class HWAccelPlayerSprite
	{
	public:
		FSoftwareTexture *pic = nullptr;
		double texturemid = 0.0;
		float yscale = 0.0f;
		fixed_t xscale = 0;

		float Alpha = 0.0f;
		FRenderStyle RenderStyle;
		FTranslationID Translation = NO_TRANSLATION; // this gets passed to the 2D code which works with high level IDs.
		uint32_t FillColor = 0;

		FDynamicColormap *basecolormap = nullptr;
		int x1 = 0;

		bool flip = false;
		FSpecialColormap *special = nullptr;
		PalEntry overlay = 0;
		PalEntry LightColor = 0xffffffff;
		uint8_t Desaturate = 0;
	};

	class RenderPlayerSprites
	{
	public:
		RenderPlayerSprites(RenderThread *thread);

		void Render();
		void RenderRemaining();

		RenderThread *Thread = nullptr;

	private:
		void RenderSprite(DPSprite *pspr, AActor *owner, float bobx, float boby, double wx, double wy, double ticfrac, int lightlevel, FDynamicColormap *basecolormap, bool foggy);

		static constexpr int BASEXCENTER = 160;
		static constexpr int BASEYCENTER = 100;

		TArray<HWAccelPlayerSprite> AcceleratedSprites;
		sector_t tempsec;
	};
}
