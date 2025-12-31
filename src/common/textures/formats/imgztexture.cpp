/*
** imgztexture.cpp
**
** Texture class for IMGZ style images
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
*/

#include "files.h"
#include "filesystem.h"
#include "bitmap.h"
#include "imagehelpers.h"
#include "image.h"

bool checkIMGZPalette(FileReader &file);

//==========================================================================
//
// An IMGZ image (mostly just crosshairs)
// [RH] Just a format I invented to avoid WinTex's palette remapping
// when I wanted to insert some alpha maps.
//
//==========================================================================

class FIMGZTexture : public FImageSource
{
	struct ImageHeader
	{
		uint8_t Magic[4];
		uint16_t Width;
		uint16_t Height;
		int16_t LeftOffset;
		int16_t TopOffset;
		uint8_t Compression;
		uint8_t Reserved[11];
	};

	bool isalpha = true;

public:
	FIMGZTexture (int lumpnum, uint16_t w, uint16_t h, int16_t l, int16_t t, bool isalpha);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override;
};


//==========================================================================
//
//
//
//==========================================================================

FImageSource *IMGZImage_TryCreate(FileReader & file, int lumpnum)
{
	uint32_t magic = 0;
	uint16_t w, h;
	int16_t l, t;
	bool ispalette;

	file.Seek(0, FileReader::SeekSet);
	if (file.Read(&magic, 4) != 4) return NULL;
	if (magic != MAKE_ID('I','M','G','Z')) return NULL;
	w = file.ReadUInt16();
	h = file.ReadUInt16();
	l = file.ReadInt16();
	t = file.ReadInt16();
	ispalette = checkIMGZPalette(file);
	return new FIMGZTexture(lumpnum, w, h, l, t, !ispalette);
}

//==========================================================================
//
//
//
//==========================================================================

FIMGZTexture::FIMGZTexture (int lumpnum, uint16_t w, uint16_t h, int16_t l, int16_t t, bool _isalpha)
	: FImageSource(lumpnum)
{
	Width = w;
	Height = h;
	LeftOffset = l;
	TopOffset = t;
	isalpha = _isalpha;
	bUseGamePalette = !isalpha;
}

//==========================================================================
//
//
//
//==========================================================================

PalettedPixels FIMGZTexture::CreatePalettedPixels(int conversion, int frame)
{
	auto lump =  fileSystem.ReadFile (SourceLump);
	auto imgz = (const ImageHeader *)lump.data();
	const uint8_t *data = (const uint8_t *)&imgz[1];

	uint8_t *dest_p;
	int dest_adv = Height;
	int dest_rew = Width * Height - 1;

	PalettedPixels Pixels(Width*Height);
	dest_p = Pixels.Data();

	const uint8_t *remap = ImageHelpers::GetRemap(conversion == luminance, isalpha);

	// Convert the source image from row-major to column-major format and remap it
	if (!imgz->Compression)
	{
		for (int y = Height; y != 0; --y)
		{
			for (int x = Width; x != 0; --x)
			{
				*dest_p = remap[*data];
				dest_p += dest_adv;
				data++;
			}
			dest_p -= dest_rew;
		}
	}
	else
	{
		// IMGZ compression is the same RLE used by IFF ILBM files
		int runlen = 0, setlen = 0;
		uint8_t setval = 0;  // Shut up, GCC

		for (int y = Height; y != 0; --y)
		{
			for (int x = Width; x != 0; )
			{
				if (runlen != 0)
				{
					*dest_p = remap[*data];
					dest_p += dest_adv;
					data++;
					x--;
					runlen--;
				}
				else if (setlen != 0)
				{
					*dest_p = setval;
					dest_p += dest_adv;
					x--;
					setlen--;
				}
				else
				{
					int8_t code = *data++;
					if (code >= 0)
					{
						runlen = code + 1;
					}
					else if (code != -128)
					{
						setlen = (-code) + 1;
						setval = remap[*data++];
					}
				}
			}
			dest_p -= dest_rew;
		}
	}
	return Pixels;
}

//==========================================================================
//
//
//
//==========================================================================

int FIMGZTexture::CopyPixels(FBitmap *bmp, int conversion, int frame)
{
	if (!isalpha) return FImageSource::CopyPixels(bmp, conversion, frame);
	else return CopyTranslatedPixels(bmp, GPalette.GrayscaleMap.Palette, frame);
}

