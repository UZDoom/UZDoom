/*
** firetexture.cpp
**
** PSX/N64-style fire texture implementation
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Cacodemon345
** Copyright 2024-2025 GZDoom Maintainers and Contributors
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

/* Algorithm based on https://github.com/fabiensanglard/DoomFirePSX */

#include "bitmap.h"
#include "firetexture.h"
#include "m_random.h"
#include "imagehelpers.h"

#include "engineerrors.h"

static constexpr int32_t FIRESKY_W = 64;
static constexpr int32_t FIRESKY_H = 128;

void FireTexture::Reset()
{
	Image.Clear();
	Image.AppendFill(0, Width * Height);
	for (size_t i = 0u; i < Width; ++i)
		Image[(Height - 1) * Width + i] = (uint8_t)(Palette.Size() - 1);
}

FireTexture::FireTexture()
{
	SetSize(FIRESKY_W, FIRESKY_H);
	Image.Clear();
	Image.AppendFill(0, Width * Height);
}

void FireTexture::SetPalette(TArray<PalEntry>& colors)
{
	Palette.Clear();
	Palette.Append(colors);

	/* This shouldn't happen in any circumstances. */
	if (Palette.Size() > 256)
	{
		I_FatalError("Fire palette too big!");
	}

	for (unsigned int i = 0; i < Width; i++) {
		Image[(Height - 1) * Width + i] = (uint8_t)(Palette.Size() - 1);
	}
}

void FireTexture::Update()
{
	for (unsigned int x = 0; x < Width; x++)
	{
		for (unsigned int y = 1; y < Height; y++)
		{
			uint8_t srcPixel = Image[y * Width + x];

			if (srcPixel == 0)
			{
				Image[(y - 1) * Width + x] = 0;
			}
			else
			{
				int XRand = M_Random() & 3;
				int destRand = M_Random() & 1;

				Image[(y - 1) * Width + ((x + 1 - XRand) % Width)] = srcPixel - destRand;
			}
		}
	}
}

FBitmap FireTexture::GetBgraBitmap(const PalEntry* remap, int* trans)
{
	FBitmap bitmap;
	bitmap.Create(Width, Height);
	uint32_t* pixels = (uint32_t*)bitmap.GetPixels();

	for (unsigned int y = 0; y < Height; y++)
	{
		for (unsigned int x = 0; x < Width; x++)
		{
			pixels[y * Width + x] = Palette[Image[y * Width + x]];
		}
	}
	
	return bitmap;
}

TArray<uint8_t> FireTexture::Get8BitPixels(bool alphatex)
{
	FBitmap bitmap = GetBgraBitmap(nullptr, nullptr);
	const uint8_t* data = bitmap.GetPixels();

	uint8_t* dest_p;
	int dest_adv = Height;
	int dest_rew = Width * Height - 1;

	TArray<uint8_t> Pixels(Width * Height);
	dest_p = Pixels.Data();

	bool doalpha = alphatex;
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
