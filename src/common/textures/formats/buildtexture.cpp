/*
** buildtexture.cpp
**
** Handling Build textures (now as a usable editing feature!)
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Marisa Heit
** Copyright 2006-2018 Christoph Oelckers
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

#include "files.h"
#include "bitmap.h"
#include "image.h"
#include "palettecontainer.h"


//==========================================================================
//
//
//
//==========================================================================

FBuildTexture::FBuildTexture(const FString &pathprefix, int tilenum, const uint8_t *pixels, FRemapTable *translation, int width, int height, int left, int top)
: RawPixels (pixels), Translation(translation)
{
	Width = width;
	Height = height;
	LeftOffset = left;
	TopOffset = top;
}

PalettedPixels FBuildTexture::CreatePalettedPixels(int conversion, int frame)
{
	PalettedPixels Pixels(Width * Height);
	FRemapTable *Remap = Translation;
	for (int i = 0; i < Width*Height; i++)
	{
		auto c = RawPixels[i];
		Pixels[i] = conversion == luminance ? Remap->Palette[c].Luminance() : Remap->Remap[c];
	}
	return Pixels;
}

int FBuildTexture::CopyPixels(FBitmap *bmp, int conversion, int frame)
{
	PalEntry *Remap = Translation->Palette;
	bmp->CopyPixelData(0, 0, RawPixels, Width, Height, Height, 1, 0, Remap);
	return -1;

}

