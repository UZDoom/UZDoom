/*
** r_drawerargs.cpp
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
#include "r_drawerargs.h"

namespace swrenderer
{
	void DrawerArgs::SetBaseColormap(FSWColormap *base_colormap)
	{
		mBaseColormap = base_colormap;
		assert(mBaseColormap->Maps != nullptr);
	}

	void DrawerArgs::SetLight(float light, int lightlevel, bool foggy, RenderViewport *viewport)
	{
		mLight = light;
		mShade = LightVisibility::LightLevelToShade(lightlevel, foggy, viewport);
	}

	void DrawerArgs::SetLight(const ColormapLight &light)
	{
		SetBaseColormap(light.BaseColormap);
		SetLight(0.0f, light.ColormapNum << FRACBITS);
	}

	void DrawerArgs::SetLight(float light, int shade)
	{
		mLight = light;
		mShade = shade;
	}

	void DrawerArgs::SetTranslationMap(lighttable_t *translation)
	{
		mTranslation = translation;
	}

	uint8_t *DrawerArgs::Colormap(RenderViewport *viewport) const
	{
		if (mBaseColormap)
		{
			if (viewport->RenderTarget->IsBgra())
				return mBaseColormap->Maps;
			else
				return mBaseColormap->Maps + (GETPALOOKUP(mLight, mShade) << COLORMAPSHIFT);
		}
		else
		{
			return mTranslation;
		}
	}

	ShadeConstants DrawerArgs::ColormapConstants() const
	{
		ShadeConstants shadeConstants;
		if (mBaseColormap)
		{
			shadeConstants.light_red = mBaseColormap->Color.r * 256 / 255;
			shadeConstants.light_green = mBaseColormap->Color.g * 256 / 255;
			shadeConstants.light_blue = mBaseColormap->Color.b * 256 / 255;
			shadeConstants.light_alpha = mBaseColormap->Color.a * 256 / 255;
			shadeConstants.fade_red = mBaseColormap->Fade.r;
			shadeConstants.fade_green = mBaseColormap->Fade.g;
			shadeConstants.fade_blue = mBaseColormap->Fade.b;
			shadeConstants.fade_alpha = mBaseColormap->Fade.a;
			shadeConstants.desaturate = min(abs(mBaseColormap->Desaturate), 255) * 255 / 256;
			shadeConstants.simple_shade = (mBaseColormap->Color.d == 0x00ffffff && mBaseColormap->Fade.d == 0x00000000 && mBaseColormap->Desaturate == 0);
		}
		else
		{
			shadeConstants.light_red = 256;
			shadeConstants.light_green = 256;
			shadeConstants.light_blue = 256;
			shadeConstants.light_alpha = 256;
			shadeConstants.fade_red = 0;
			shadeConstants.fade_green = 0;
			shadeConstants.fade_blue = 0;
			shadeConstants.fade_alpha = 256;
			shadeConstants.desaturate = 0;
			shadeConstants.simple_shade = true;
		}
		return shadeConstants;
	}
}
