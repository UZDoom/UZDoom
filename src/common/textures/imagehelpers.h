/*
** imagehelpers.h
**
** Utilities for image conversion - mostly 8 bit paletted baggage
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Marisa Heit
** Copyright 2005-2018 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <stdint.h>
#include "tarray.h"
#include "colormatcher.h"
#include "bitmap.h"
#include "palettecontainer.h"
#include "v_colortables.h"

namespace ImageHelpers
{
	// Helpers for creating paletted images.
	inline uint8_t *GetRemap(bool wantluminance, bool srcisgrayscale = false)
	{
		if (wantluminance)
		{
			return srcisgrayscale ? GPalette.GrayRamp.Remap : GPalette.GrayscaleMap.Remap;
		}
		else
		{
			return srcisgrayscale ? GPalette.GrayMap : GPalette.Remap;
		}
	}

	inline uint8_t RGBToPalettePrecise(bool wantluminance, int r, int g, int b, int a = 255)
	{
		if (wantluminance)
		{
			return (uint8_t)Luminance(r, g, b) * a / 255;
		}
		else
		{
			return ColorMatcher.Pick(r, g, b);
		}
	}

	inline uint8_t RGBToPalette(bool wantluminance, int r, int g, int b, int a = 255)
	{
		if (wantluminance)
		{
			// This is the same formula the OpenGL renderer uses for grayscale textures with an alpha channel.
			return (uint8_t)(Luminance(r, g, b) * a / 255);
		}
		else
		{
			return a < 128? 0 : RGB256k.RGB[r >> 2][g >> 2][b >> 2];
		}
	}

	inline uint8_t RGBToPalette(bool wantluminance, PalEntry pe, bool hasalpha = true)
	{
		return RGBToPalette(wantluminance, pe.r, pe.g, pe.b, hasalpha? pe.a : 255);
	}

	//==========================================================================
	//
	// Converts a texture between row-major and column-major format
	// by flipping it about the X=Y axis.
	//
	//==========================================================================

	template<class T>
	void FlipSquareBlock (T *block, int x)
	{
		for (int i = 0; i < x; ++i)
		{
			T *corner = block + x*i + i;
			int count = x - i;
			for (int j = 0; j < count; j++)
			{
				std::swap(corner[j], corner[j*x]);
			}
		}
	}

	inline void FlipSquareBlockRemap (uint8_t *block, int x, const uint8_t *remap)
	{
		for (int i = 0; i < x; ++i)
		{
			uint8_t *corner = block + x*i + i;
			int count = x - i;
			for (int j = 0; j < count; j++)
			{
				auto t = remap[corner[j]];
				corner[j] = remap[corner[j*x]];
				corner[j*x] = t;
			}
		}
	}

	template<class T>
	void FlipNonSquareBlock (T *dst, const T *src, int x, int y, int srcpitch)
	{
		for (int i = 0; i < x; ++i)
		{
			for (int j = 0; j < y; ++j)
			{
				dst[i*y+j] = src[i+j*srcpitch];
			}
		}
	}

	inline void FlipNonSquareBlockRemap (uint8_t *dst, const uint8_t *src, int x, int y, int srcpitch, const uint8_t *remap)
	{
		for (int i = 0; i < x; ++i)
		{
			for (int j = 0; j < y; ++j)
			{
				dst[i*y+j] = remap[src[i+j*srcpitch]];
			}
		}
	}
}
