/*
** emptytexture.cpp
**
** Texture class for empty placeholder textures
** (essentially patches with dimensions and offsets of (0,0) )
** These need special treatment because a texture size of 0 is illegal
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Christoph Oelckers
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
#include "filesystem.h"
#include "image.h"

//==========================================================================
//
// 
//
//==========================================================================

class FEmptyTexture : public FImageSource
{
public:
	FEmptyTexture (int lumpnum);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
};

//==========================================================================
//
// 
//
//==========================================================================

FImageSource *EmptyImage_TryCreate(FileReader & file, int lumpnum)
{
	char check[8];
	if (file.GetLength() != 8) return NULL;
	file.Seek(0, FileReader::SeekSet);
	if (file.Read(check, 8) != 8) return NULL;
	if (memcmp(check, "\0\0\0\0\0\0\0\0", 8)) return NULL;

	return new FEmptyTexture(lumpnum);
}

FImageSource* CreateEmptyTexture()
{
	return new FEmptyTexture(0);
}

//==========================================================================
//
//
//
//==========================================================================

FEmptyTexture::FEmptyTexture (int lumpnum)
: FImageSource(lumpnum)
{
	bMasked = true;
	Width = Height = 1;
	bUseGamePalette = true;
}

//==========================================================================
//
//
//
//==========================================================================

PalettedPixels FEmptyTexture::CreatePalettedPixels(int conversion, int frame)
{
	static uint8_t p;
	PalettedPixels Pixel(&p, 1);
	return Pixel;
}

