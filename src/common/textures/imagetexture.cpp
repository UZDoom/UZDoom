/*
** imagetexture.cpp
**
** Texture class based on FImageSource
**
**---------------------------------------------------------------------------
**
** Copyright 2018 Christoph Oelckers
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

#include "bitmap.h"
#include "image.h"
#include "textures.h"


//==========================================================================
//
//
//
//==========================================================================

FImageTexture::FImageTexture(FImageSource *img, int frame) noexcept
: FTexture(img? img->LumpNum() : 0)
{
	mImage = img;
	TexFrame = frame;
	if (img != nullptr)
	{
		SetFromImage();
	}
}

FImageTexture::~FImageTexture()
{
	delete mImage;
}

void FImageTexture::SetFromImage()
{
	auto img = mImage;
	Width = img->GetWidth();
	Height = img->GetHeight();

	Masked = img->bMasked;
	bTranslucent = img->bTranslucent;
}
//===========================================================================
//
// 
//
//===========================================================================

FBitmap FImageTexture::GetBgraBitmap(const PalEntry *p, int *trans)
{
	return mImage->GetCachedBitmap(p, bNoRemap0? FImageSource::noremap0 : FImageSource::normal, trans, TexFrame);
}	

//===========================================================================
//
// 
//
//===========================================================================

TArray<uint8_t> FImageTexture::Get8BitPixels(bool alpha)
{
	return mImage->GetPalettedPixels(alpha? FImageSource::luminance : bNoRemap0 ? FImageSource::noremap0 : FImageSource::normal, TexFrame);
}	

//===========================================================================
//
// use the already known state of the underlying image to save time.
//
//===========================================================================

bool FImageTexture::DetermineTranslucency()
{
	if (mImage->bTranslucent != -1)
	{
		bTranslucent = mImage->bTranslucent;
		return !!bTranslucent;
	}
	else
	{
		return FTexture::DetermineTranslucency();
	}
}


FTexture* CreateImageTexture(FImageSource* img, int frame) noexcept
{
	return new FImageTexture(img, frame);
}

