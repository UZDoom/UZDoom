/*
** brightmaptexture.cpp
**
** The texture class for colormap based brightmaps.
**
**---------------------------------------------------------------------------
**
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

#include "filesystem.h"
#include "palettecontainer.h"
#include "bitmap.h"
#include "image.h"

class FBrightmapTexture : public FImageSource
{
public:
	FBrightmapTexture (FImageSource *source);

	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override;

protected:
	FImageSource *SourcePic;
};

//===========================================================================
//
// fake brightness maps
// These are generated for textures affected by a colormap with
// fullbright entries.
//
//===========================================================================

FBrightmapTexture::FBrightmapTexture (FImageSource *source)
{
	SourcePic = source;
	Width = source->GetWidth();
	Height = source->GetHeight();
	bMasked = false;
}

int FBrightmapTexture::CopyPixels(FBitmap *bmp, int conversion, int frame)
{
	SourcePic->CopyTranslatedPixels(bmp, GPalette.GlobalBrightmap.Palette, frame);
	return 0;
}

FTexture *CreateBrightmapTexture(FImageSource *tex)
{
	return CreateImageTexture(new FBrightmapTexture(tex));
}
