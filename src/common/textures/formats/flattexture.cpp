/*
** flattexture.cpp
**
** Texture class for standard Doom flats
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
**
*/

#include "files.h"
#include "filesystem.h"
#include "imagehelpers.h"
#include "image.h"

//==========================================================================
//
// A texture defined between F_START and F_END markers
//
//==========================================================================

class FFlatTexture : public FImageSource
{
public:
	FFlatTexture (int lumpnum);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
};



//==========================================================================
//
// Since there is no way to detect the validity of a flat
// they can't be used anywhere else but between F_START and F_END
//
//==========================================================================

FImageSource *FlatImage_TryCreate(FileReader & file, int lumpnum)
{
	return new FFlatTexture(lumpnum);
}

//==========================================================================
//
//
//
//==========================================================================

FFlatTexture::FFlatTexture (int lumpnum)
: FImageSource(lumpnum)
{
	int bits;

	auto area = fileSystem.FileLength (lumpnum);

	switch (area)
	{
	default:
	case 64*64:		bits = 6;	break;
	case 8*8:		bits = 3;	break;
	case 16*16:		bits = 4;	break;
	case 32*32:		bits = 5;	break;
	case 128*128:	bits = 7;	break;
	case 256*256:	bits = 8;	break;
	}

	bUseGamePalette = true;
	bMasked = false;
	bTranslucent = false;
	Width = Height = 1 << bits;
}

//==========================================================================
//
//
//
//==========================================================================

PalettedPixels FFlatTexture::CreatePalettedPixels(int conversion, int frame)
{
	auto lump = fileSystem.OpenFileReader (SourceLump);
	PalettedPixels Pixels(Width*Height);
	auto numread = lump.Read (Pixels.Data(), Width*Height);
	if (numread < Width*Height)
	{
		memset (Pixels.Data() + numread, 0xBB, Width*Height - numread);
	}
	ImageHelpers::FlipSquareBlockRemap(Pixels.Data(), Width, ImageHelpers::GetRemap(conversion == luminance));
	return Pixels;
}

