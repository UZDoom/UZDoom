/*
** anmtexture.cpp
**
** Texture class for reading the first frame of Build ANM files
**
**---------------------------------------------------------------------------
**
** Copyright 2020 Christoph Oelckers
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

#include <memory>
#include "files.h"
#include "filesystem.h"
#include "bitmap.h"
#include "imagehelpers.h"
#include "image.h"
#include "animlib.h"

//==========================================================================
//
//
//==========================================================================

class FAnmTexture : public FImageSource
{

public:
	FAnmTexture (int lumpnum, int w, int h);
	void ReadFrame(uint8_t *buffer, uint8_t *palette);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override;
};


//==========================================================================
//
//
//
//==========================================================================

FImageSource *AnmImage_TryCreate(FileReader & file, int lumpnum)
{
	file.Seek(0, FileReader::SeekSet);
	char check[4];
	auto num = file.Read(check, 4);
	if (num < 4) return nullptr;
	if (memcmp(check, "LPF ", 4)) return nullptr;
	file.Seek(0, FileReader::SeekSet);
	auto buffer = file.ReadPadded(1);
	if (buffer.size() < 4) return nullptr;

	std::unique_ptr<anim_t> anim = std::make_unique<anim_t>(); // note that this struct is very large and should not go onto the stack!
	if (ANIM_LoadAnim(anim.get(), buffer.bytes(), buffer.size() - 1) < 0)
	{
		return nullptr;
	}
	int numframes = ANIM_NumFrames(anim.get());
	if (numframes >= 1)
	{
		return new FAnmTexture(lumpnum, 320, 200);
	}

	return nullptr;
}

//==========================================================================
//
//
//
//==========================================================================

FAnmTexture::FAnmTexture (int lumpnum, int w, int h)
	: FImageSource(lumpnum)
{
	Width = w;
	Height = h;
	LeftOffset = 0;
	TopOffset = 0;
}

void FAnmTexture::ReadFrame(uint8_t *pixels, uint8_t *palette)
{
	auto lump = fileSystem.ReadFile (SourceLump);
	auto source = lump.bytes(); 

	std::unique_ptr<anim_t> anim = std::make_unique<anim_t>(); // note that this struct is very large and should not go onto the stack!
	if (ANIM_LoadAnim(anim.get(), source, (int)lump.size()) >= 0)
	{
		int numframes = ANIM_NumFrames(anim.get());
		if (numframes >= 1)
		{
			memcpy(palette, ANIM_GetPalette(anim.get()), 768);
			memcpy(pixels, ANIM_DrawFrame(anim.get(), 1), Width * Height);
			return;
		}
	}
	memset(pixels, 0, Width*Height);
	memset(palette, 0, 768);
}

struct workbuf
{
	uint8_t buffer[64000];
	uint8_t palette[768];
};

//==========================================================================
//
//
//
//==========================================================================

PalettedPixels FAnmTexture::CreatePalettedPixels(int conversion, int frame)
{
	PalettedPixels pixels(Width*Height);
	uint8_t remap[256];
	std::unique_ptr<workbuf> w = std::make_unique<workbuf>();

	ReadFrame(w->buffer, w->palette);
	for(int i=0;i<256;i++)
	{
		remap[i] = ColorMatcher.Pick(w->palette[i*3], w->palette[i*3+1], w->palette[i*3+2]);
	}
	ImageHelpers::FlipNonSquareBlockRemap (pixels.Data(), w->buffer, Width, Height, Width, remap); 
	return pixels;
}

//==========================================================================
//
//
//
//==========================================================================

int FAnmTexture::CopyPixels(FBitmap *bmp, int conversion, int frame)
{
	std::unique_ptr<workbuf> w = std::make_unique<workbuf>();
	ReadFrame(w->buffer, w->palette);

    auto dpix = bmp->GetPixels();
	for (int i = 0; i < Width * Height; i++)
	{
		int p = i * 4;
		int index = w->buffer[i];
		dpix[p + 0] = w->palette[index * 3 + 2];
		dpix[p + 1] = w->palette[index * 3 + 1];
		dpix[p + 2] = w->palette[index * 3];
		dpix[p + 3] = 255;
	}

	return -1;
}

 
