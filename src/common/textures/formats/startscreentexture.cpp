/*
** startscreentexture.cpp
**
** Texture class to create a texture from the start screen's imagé
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Marisa Heit
** Copyright 2019 Christoph Oelckers
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
#include "textures.h"
#include "imagehelpers.h"
#include "image.h"
#include "startscreen.h"


//==========================================================================
//
//
//
//==========================================================================

class FStartScreenTexture : public FImageSource
{
	FBitmap& info; // This must remain constant for the lifetime of this texture

public:
	FStartScreenTexture(FBitmap& srcdata)
		: FImageSource(-1), info(srcdata)
	{
		Width = srcdata.GetWidth();
		Height = srcdata.GetHeight();
		bUseGamePalette = false;
	}
	int CopyPixels(FBitmap* bmp, int conversion, int frame = 0) override
	{
		bmp->Blit(0, 0, info);
		return 0;
	}
};

//==========================================================================
//
//
//
//==========================================================================

FImageSource *CreateStartScreenTexture(FBitmap& srcdata)
{
	return new FStartScreenTexture(srcdata);
}


