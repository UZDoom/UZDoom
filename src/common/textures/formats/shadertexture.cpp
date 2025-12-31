/*
** shadertexture.cpp
**
** simple shader gradient textures, used by the status bars.
**
**---------------------------------------------------------------------------
**
** Copyright 2008 Braden Obrzut
** Copyright 2017 Christoph Oelckers
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
#include "imagehelpers.h"
#include "image.h"
#include "textures.h"


class FBarShader : public FImageSource
{
public:
	FBarShader(bool vertical, bool reverse)
	{
		int i;

		Width = vertical ? 2 : 256;
		Height = vertical ? 256 : 2;
		bMasked = false;
		bTranslucent = false;

		// Fill the column/row with shading values.
		// Vertical shaders have have minimum alpha at the top
		// and maximum alpha at the bottom, unless flipped by
		// setting reverse to true. Horizontal shaders are just
		// the opposite.
		if (vertical)
		{
			if (!reverse)
			{
				for (i = 0; i < 256; ++i)
				{
					Pixels[i] = i;
					Pixels[256+i] = i;
				}
			}
			else
			{
				for (i = 0; i < 256; ++i)
				{
					Pixels[i] = 255 - i;
					Pixels[256+i] = 255 -i;
				}
			}
		}
		else
		{
			if (!reverse)
			{
				for (i = 0; i < 256; ++i)
				{
					Pixels[i*2] = 255 - i;
					Pixels[i*2+1] = 255 - i;
				}
			}
			else
			{
				for (i = 0; i < 256; ++i)
				{
					Pixels[i*2] = i;
					Pixels[i*2+1] = i;
				}
			}
		}
	}

	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override
	{
		PalettedPixels Pix(512);
		if (conversion == luminance)
		{
			memcpy(Pix.Data(), Pixels, 512);
		}
		else
		{
			// Since this presents itself to the game as a regular named texture
			// it can easily be used on walls and flats and should work as such, 
			// even if it makes little sense.
			for (int i = 0; i < 512; i++)
			{
				Pix[i] = GPalette.GrayMap[Pixels[i]];
			}
		}
		return Pix;
	}

	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override
	{
		bmp->CopyPixelData(0, 0, Pixels, Width, Height, Height, 1, 0, GPalette.GrayRamp.Palette);
		return 0;
	}

private:
	uint8_t Pixels[512];
};


FGameTexture *CreateShaderTexture(bool vertical, bool reverse)
{
	FStringf name("BarShader%c%c", vertical ? 'v' : 'h', reverse ? 'r' : 'f');
	return MakeGameTexture(CreateImageTexture(new FBarShader(vertical, reverse)), name.GetChars(), ETextureType::Override);

}
