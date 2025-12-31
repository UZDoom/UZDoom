/*
** rawpagetexture.cpp
**
** Texture class for Raven's raw fullscreen pages
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
#include "m_swap.h"

// Doom patch format header
struct patch_t
{
	int16_t			width;			// bounding box size 
	int16_t			height;
	int16_t			leftoffset; 	// pixels to the left of origin 
	int16_t			topoffset;		// pixels below the origin 
	uint32_t 		columnofs[1];	// only [width] used
};


//==========================================================================
//
// A raw 320x200 graphic used by Heretic and Hexen fullscreen images
//
//==========================================================================

class FRawPageTexture : public FImageSource
{
	int mPaletteLump = -1;
public:
	FRawPageTexture (int lumpnum);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override;
};

//==========================================================================
//
// RAW textures must be exactly 64000 bytes long and not be identifiable
// as Doom patch
//
//==========================================================================

bool CheckIfRaw(FileReader & data, unsigned desiredsize)
{
	if (data.GetLength() != desiredsize) return false;

	// This is probably a raw page graphic, but do some checking to be sure
	patch_t *foo;
	int height;
	int width;

	data.Seek(0, FileReader::SeekSet);
	auto bits = data.Read(data.GetLength());
	foo = (patch_t *)bits.data();

	height = LittleShort(foo->height);
	width = LittleShort(foo->width);

	if (height > 0 && height < 510 && width > 0 && width < 15997)
	{
		// The dimensions seem like they might be valid for a patch, so
		// check the column directory for extra security. At least one
		// column must begin exactly at the end of the column directory,
		// and none of them must point past the end of the patch.
		bool gapAtStart = true;
		int x;

		for (x = 0; x < width; ++x)
		{
			uint32_t ofs = LittleLong(foo->columnofs[x]);
			if (ofs == (uint32_t)width * 4 + 8)
			{
				gapAtStart = false;
			}
			else if (ofs >= desiredsize-1)	// Need one byte for an empty column
			{
				return true;
			}
			else
			{
				// Ensure this column does not extend beyond the end of the patch
				const uint8_t *foo2 = (const uint8_t *)foo;
				while (ofs < desiredsize)
				{
					if (foo2[ofs] == 255)
					{
						return true;
					}
					ofs += foo2[ofs+1] + 4;
				}
				if (ofs >= desiredsize)
				{
					return true;
				}
			}
		}
		if (gapAtStart || (x != width))
		{
			return true;
		}
		return false;
	}
	else
	{
		return true;
	}
}

//==========================================================================
//
//
//
//==========================================================================

FImageSource *RawPageImage_TryCreate(FileReader & file, int lumpnum)
{
	if (!CheckIfRaw(file, 64000)) return nullptr;
	return new FRawPageTexture(lumpnum);
}


//==========================================================================
//
//
//
//==========================================================================

FRawPageTexture::FRawPageTexture (int lumpnum)
: FImageSource(lumpnum)
{
	Width = 320;
	Height = 200;

	// Special case hack for Heretic's E2 end pic. This is not going to be exposed as an editing feature because the implications would be horrible.
	auto Name = fileSystem.GetFileShortName(lumpnum);
	if (stricmp(Name, "E2END") == 0)
	{
		mPaletteLump = fileSystem.CheckNumForName("E2PAL");
		if (fileSystem.FileLength(mPaletteLump) < 768) mPaletteLump = -1;
	}
	else bUseGamePalette = true;
}

//==========================================================================
//
//
//
//==========================================================================

PalettedPixels FRawPageTexture::CreatePalettedPixels(int conversion, int frame)
{
	auto lump =  fileSystem.ReadFile (SourceLump);
	auto source = lump.bytes();
	const uint8_t *source_p = source;
	uint8_t *dest_p;

	PalettedPixels Pixels(Width*Height);
	dest_p = Pixels.Data();

	const uint8_t *remap = ImageHelpers::GetRemap(conversion == luminance);

	// This does not handle the custom palette. 
	// User maps are encouraged to use a real image format when replacing E2END and the original could never be used anywhere else.

	// Convert the source image from row-major to column-major format
	for (int y = 200; y != 0; --y)
	{
		for (int x = 320; x != 0; --x)
		{
			*dest_p = remap[*source_p];
			dest_p += 200;
			source_p++;
		}
		dest_p -= 200 * 320 - 1;
	}
	return Pixels;
}

int FRawPageTexture::CopyPixels(FBitmap *bmp, int conversion, int frame)
{
	if (mPaletteLump < 0) return FImageSource::CopyPixels(bmp, conversion, frame);
	else
	{
		auto lump =  fileSystem.ReadFile(SourceLump);
		auto plump = fileSystem.ReadFile(mPaletteLump);
		auto source = lump.bytes();
		auto psource = plump.bytes();
		PalEntry paldata[256];
		for (auto & pe : paldata)
		{
			pe.r = *psource++;
			pe.g = *psource++;
			pe.b = *psource++;
			pe.a = 255;
		}
		bmp->CopyPixelData(0, 0, source, 320, 200, 1, 320, 0, paldata);
	}
	return 0;
}
