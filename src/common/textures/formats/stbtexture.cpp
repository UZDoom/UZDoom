/*
** stbtexture.cpp
**
** Texture class for reading textures with stb_image
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Christoph Oelckers
** Copyright 2019-2025 GZDoom Maintainers and Contributors
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

#define STB_IMAGE_IMPLEMENTATION    
#define STBI_NO_STDIO
// Undefine formats we do not want to support here.
//#define STBI_NO_PNG we need PNG for 16 bit channel images. Regular ones still use our own, more flexible decoder.
#define STBI_NO_TGA // we could use that but our own loader has better palette support.
#define STBI_NO_PSD
#define STBI_NO_HDR
#define STBI_NO_PNM
#include "stb_image.h"


#include "files.h"
#include "filesystem.h"
#include "bitmap.h"
#include "imagehelpers.h"
#include "image.h"

//==========================================================================
//
// A texture backed by stb_image
// This will load GIF, BMP and PICT images.
// PNG, JPG and TGA are still being handled by the existing dedicated implementations.
// PSD and HDR are impractical for reading texture data and thus are disabled.
// PnM could be enabled, if its identification semantics were stronger.
// stb_image only checks the first two characters which simply would falsely identify several flats with the right colors in the first two bytes.
//
//==========================================================================

class FStbTexture : public FImageSource
{

public:
	FStbTexture (int lumpnum, int w, int h);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override;
};


static stbi_io_callbacks callbacks = {
	[](void* user,char* data,int size) -> int { return (int)reinterpret_cast<FileReader*>(user)->Read(data, size); },
	[](void* user,int n) { reinterpret_cast<FileReader*>(user)->Seek(n, FileReader::SeekCur); },
	[](void* user) -> int { return reinterpret_cast<FileReader*>(user)->Tell() >= reinterpret_cast<FileReader*>(user)->GetLength(); }
};
//==========================================================================
//
//
//
//==========================================================================

FImageSource *StbImage_TryCreate(FileReader & file, int lumpnum)
{
	int x, y, comp;
	file.Seek(0, FileReader::SeekSet);
	int result = stbi_info_from_callbacks(&callbacks, &file, &x, &y, &comp);
	if (result == 1)
	{
		return new FStbTexture(lumpnum, x, y);
	}

	return nullptr;
}

//==========================================================================
//
//
//
//==========================================================================

FStbTexture::FStbTexture (int lumpnum, int w, int h)
	: FImageSource(lumpnum)
{
	Width = w;
	Height = h;
	LeftOffset = 0;
	TopOffset = 0;
}

//==========================================================================
//
//
//
//==========================================================================

PalettedPixels FStbTexture::CreatePalettedPixels(int conversion, int frame)
{
	FBitmap bitmap;
	bitmap.Create(Width, Height);
	CopyPixels(&bitmap, conversion);
	const uint8_t *data = bitmap.GetPixels();

	uint8_t *dest_p;
	int dest_adv = Height;
	int dest_rew = Width * Height - 1;

	PalettedPixels Pixels(Width*Height);
	dest_p = Pixels.Data();

	bool doalpha = conversion == luminance; 
	// Convert the source image from row-major to column-major format and remap it
	for (int y = Height; y != 0; --y)
	{
		for (int x = Width; x != 0; --x)
		{
			int b = *data++;
			int g = *data++;
			int r = *data++;
			int a = *data++;
			if (a < 128) *dest_p = 0;
			else *dest_p = ImageHelpers::RGBToPalette(doalpha, r, g, b); 
			dest_p += dest_adv;
		}
		dest_p -= dest_rew;
	}
	return Pixels;
}

//==========================================================================
//
//
//
//==========================================================================

int FStbTexture::CopyPixels(FBitmap *bmp, int conversion, int frame)
{
	auto lump = fileSystem.OpenFileReader (SourceLump); 
	int x, y, chan;
	auto image = stbi_load_from_callbacks(&callbacks, &lump, &x, &y, &chan, STBI_rgb_alpha); 	
	if (image)
		bmp->CopyPixelDataRGB(0, 0, image, x, y, 4, x*4, 0, CF_RGBA); 	
	stbi_image_free(image);	
	return -1;
}

 
