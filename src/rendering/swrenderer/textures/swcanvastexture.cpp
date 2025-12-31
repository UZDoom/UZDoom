/*
** swcanvastexture.cpp
**
** The base texture class
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

#include "r_swtexture.h"
#include "bitmap.h"
#include "m_alloc.h"
#include "imagehelpers.h"

static TMap<FCanvasTexture*, FSWCanvasTexture*> canvasMap;

FSWCanvasTexture* GetSWCamTex(FCanvasTexture* camtex)
{
	auto p = canvasMap.CheckKey(camtex);
	return p ? *p : nullptr;
}

FSWCanvasTexture::FSWCanvasTexture(FGameTexture* source) : FSoftwareTexture(source) 
{
	// The SW renderer needs to link the canvas textures, but let's do that outside the texture manager.
	auto camtex = static_cast<FCanvasTexture*>(source->GetTexture());
	canvasMap.Insert(camtex, this);
}


FSWCanvasTexture::~FSWCanvasTexture()
{
	if (Canvas != nullptr)
	{
		delete Canvas;
		Canvas = nullptr;
	}

	if (CanvasBgra != nullptr)
	{
		delete CanvasBgra;
		CanvasBgra = nullptr;
	}
}


//==========================================================================
//
// 
//
//==========================================================================

const uint8_t *FSWCanvasTexture::GetPixelsLocked(int style)
{
	static_cast<FCanvasTexture*>(mSource)->NeedUpdate();
	if (Canvas == nullptr)
	{
		MakeTexture();
	}
	return Pixels.Data();

}

//==========================================================================
//
//
//
//==========================================================================

const uint32_t *FSWCanvasTexture::GetPixelsBgraLocked()
{
	static_cast<FCanvasTexture*>(mSource)->NeedUpdate();
	if (CanvasBgra == nullptr)
	{
		MakeTextureBgra();
	}
	return PixelsBgra.Data();
}

//==========================================================================
//
//
//
//==========================================================================

void FSWCanvasTexture::MakeTexture ()
{
	Canvas = new DCanvas (GetWidth(), GetHeight(), false);
	Pixels.Resize(GetWidth() * GetHeight());

	// Draw a special "unrendered" initial texture into the buffer.
	memset (Pixels.Data(), 0, GetWidth() * GetHeight() / 2);
	memset (Pixels.Data() + GetWidth() * GetHeight() / 2, 255, GetWidth() * GetHeight() / 2);
}

//==========================================================================
//
//
//
//==========================================================================

void FSWCanvasTexture::MakeTextureBgra()
{
	CanvasBgra =  new DCanvas(GetWidth(), GetHeight(), true);
	PixelsBgra.Resize(GetWidth() * GetHeight());

	// Draw a special "unrendered" initial texture into the buffer.
	memset(PixelsBgra.Data(), 0, 4* GetWidth() * GetHeight() / 2);
	memset(PixelsBgra.Data() + GetWidth() * GetHeight() / 2, 255, 4* GetWidth() * GetHeight() / 2);
}

//==========================================================================
//
//
//
//==========================================================================

void FSWCanvasTexture::Unload ()
{
	if (Canvas != nullptr)
	{
		delete Canvas;
		Canvas = nullptr;
	}

	if (CanvasBgra != nullptr)
	{
		delete CanvasBgra;
		CanvasBgra = nullptr;
	}

	FSoftwareTexture::Unload();
}

//==========================================================================
//
//
//
//==========================================================================

void FSWCanvasTexture::UpdatePixels(bool truecolor)
{

	if (truecolor)
	{
		ImageHelpers::FlipNonSquareBlock(PixelsBgra.Data(), (const uint32_t*)CanvasBgra->GetPixels(), GetWidth(), GetHeight(), CanvasBgra->GetPitch());
		// True color render still sometimes uses palette textures (for sprites, mostly).
		// We need to make sure that both pixel buffers contain data:
		int width = GetWidth();
		int height = GetHeight();
		uint8_t* palbuffer = const_cast<uint8_t*>(GetPixels(0));
		const uint32_t* bgrabuffer = GetPixelsBgra();
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				uint32_t color = bgrabuffer[y];
				int r = RPART(color);
				int g = GPART(color);
				int b = BPART(color);
				palbuffer[y] = RGB32k.RGB[r >> 3][g >> 3][b >> 3];
			}
			palbuffer += height;
			bgrabuffer += height;
		}
	}
	else
	{
		ImageHelpers::FlipNonSquareBlockRemap(Pixels.Data(), Canvas->GetPixels(), GetWidth(), GetHeight(), Canvas->GetPitch(), GPalette.Remap);
	}

	static_cast<FCanvasTexture*>(mSource)->SetUpdated(false);
}
