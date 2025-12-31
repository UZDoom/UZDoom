/*
** fontchars.cpp
**
** Texture class for font characters
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

#include "filesystem.h"
#include "bitmap.h"
#include "image.h"
#include "imagehelpers.h"
#include "fontchars.h"
#include "engineerrors.h"

//==========================================================================
//
// FFontChar2 :: FFontChar2
//
// Used by FON1 and FON2 fonts.
//
//==========================================================================

FFontChar2::FFontChar2(int sourcelump, int sourcepos, int width, int height, int leftofs, int topofs)
	: SourceLump(sourcelump), SourcePos(sourcepos)
{
	Width = width;
	Height = height;
	LeftOffset = leftofs;
	TopOffset = topofs;
}

//==========================================================================
//
// FFontChar2 :: Get8BitPixels
//
// Like for FontChar1, the render style has no relevance here as well.
//
//==========================================================================

PalettedPixels FFontChar2::CreatePalettedPixels(int, int)
{
	auto lump = fileSystem.OpenFileReader(SourceLump);
	int destSize = Width * Height;
	uint8_t max = 255;
	bool rle = true;

	// This is to "fix" bad fonts
	{
		uint8_t buff[16];
		lump.Read(buff, 4);
		if (buff[3] == '2')
		{
			lump.Read(buff, 7);
			max = buff[6];
			lump.Seek(SourcePos - 11, FileReader::SeekCur);
		}
		else if (buff[3] == 0x1A)
		{
			lump.Read(buff, 13);
			max = buff[12] - 1;
			lump.Seek(SourcePos - 17, FileReader::SeekCur);
			rle = false;
		}
		else
		{
			lump.Seek(SourcePos - 4, FileReader::SeekCur);
		}
	}

	PalettedPixels Pixels(destSize);

	int runlen = 0, setlen = 0;
	uint8_t setval = 0;  // Shut up, GCC!
	uint8_t* dest_p = Pixels.Data();
	int dest_adv = Height;
	int dest_rew = destSize - 1;

	if (rle)
	{
		for (int y = Height; y != 0; --y)
		{
			for (int x = Width; x != 0; )
			{
				if (runlen != 0)
				{
					uint8_t color = lump.ReadUInt8();
					color = min(color, max);
					*dest_p = color;
					dest_p += dest_adv;
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
					int8_t code = lump.ReadInt8();
					if (code >= 0)
					{
						runlen = code + 1;
					}
					else if (code != -128)
					{
						uint8_t color = lump.ReadUInt8();
						setlen = (-code) + 1;
						setval = min(color, max);
					}
				}
			}
			dest_p -= dest_rew;
		}
	}
	else
	{
		for (int y = Height; y != 0; --y)
		{
			for (int x = Width; x != 0; --x)
			{
				uint8_t color = lump.ReadUInt8();
				if (color > max)
				{
					color = max;
				}
				*dest_p = color;
				dest_p += dest_adv;
			}
			dest_p -= dest_rew;
		}
	}

	if (destSize < 0)
	{
		auto name = fileSystem.GetFileShortName(SourceLump);
		I_FatalError("The font %s is corrupt", name);
	}
	return Pixels;
}

int FFontChar2::CopyPixels(FBitmap* bmp, int conversion, int frame)
{
	if (conversion == luminance) conversion = normal;	// luminance images have no use as an RGB source.
	auto ppix = CreatePalettedPixels(conversion);
	bmp->CopyPixelData(0, 0, ppix.Data(), Width, Height, Height, 1, 0, SourceRemap, nullptr);
	return 0;
}
