/*
** automaptexture.cpp
**
** Texture class for Raven's automap parchment
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Marisa Heit
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
** This texture type is only used as a last resort when everything else has failed for creating 
** the AUTOPAGE texture. That's because Raven used a raw lump of non-standard proportions to define it.
**
*/

#include "files.h"
#include "filesystem.h"
#include "imagehelpers.h"
#include "image.h"

//==========================================================================
//
// A raw 320x? graphic used by Heretic and Hexen for the automap parchment
//
//==========================================================================

class FAutomapTexture : public FImageSource
{
public:
	FAutomapTexture(int lumpnum);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
};



//==========================================================================
//
// This texture type will only be used for the AUTOPAGE lump if no other
// format matches.
//
//==========================================================================

FImageSource *AutomapImage_TryCreate(FileReader &data, int lumpnum)
{
	if (data.GetLength() < 320) return nullptr;
	if (!fileSystem.CheckFileName(lumpnum, "AUTOPAGE")) return nullptr;
	return new FAutomapTexture(lumpnum);
}

//==========================================================================
//
//
//
//==========================================================================

FAutomapTexture::FAutomapTexture (int lumpnum)
: FImageSource(lumpnum)
{
	Width = 320;
	Height = uint16_t(fileSystem.FileLength(lumpnum) / 320);
	bUseGamePalette = true;
}

//==========================================================================
//
//
//
//==========================================================================

PalettedPixels FAutomapTexture::CreatePalettedPixels(int conversion, int frame)
{
	int x, y;
	auto data = fileSystem.ReadFile (SourceLump);
	auto indata = data.bytes();

	PalettedPixels Pixels(Width * Height);

	const uint8_t *remap = ImageHelpers::GetRemap(conversion == luminance);
	for (x = 0; x < Width; ++x)
	{
		for (y = 0; y < Height; ++y)
		{
			auto p = indata[x + 320 * y];
			Pixels[x*Height + y] = remap[p];
		}
	}
	return Pixels;
}

