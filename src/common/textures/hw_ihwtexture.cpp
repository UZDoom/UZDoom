/*
** hw_ihwtexture.cpp
**
** Interface for hardware textures
**
**---------------------------------------------------------------------------
**
** Copyright 2006-2020 Christoph Oelckers
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

#include "hw_ihwtexture.h"
#include "basics.h"
#include "tarray.h"
#include "xs_Float.h"

//===========================================================================
// 
//	Quick'n dirty image rescaling.
//
// This will only be used when the source texture is larger than
// what the hardware can manage (extremely rare in Doom)
//
// Code taken from wxWidgets
//
//===========================================================================

struct BoxPrecalc
{
	int boxStart;
	int boxEnd;
};

static void ResampleBoxPrecalc(TArray<BoxPrecalc>& boxes, int oldDim)
{
	int newDim = boxes.Size();
	const double scale_factor_1 = double(oldDim) / newDim;
	const int scale_factor_2 = (int)(scale_factor_1 / 2);

	for (int dst = 0; dst < newDim; ++dst)
	{
		// Source pixel in the Y direction
		const int src_p = int(dst * scale_factor_1);

		BoxPrecalc& precalc = boxes[dst];
		precalc.boxStart = std::clamp<int>(int(src_p - scale_factor_1 / 2.0 + 1), 0, oldDim - 1);
		precalc.boxEnd = std::clamp<int>(std::max<int>(precalc.boxStart + 1, int(src_p + scale_factor_2)), 0, oldDim - 1);
	}
}

void IHardwareTexture::Resize(int swidth, int sheight, int width, int height, unsigned char *src_data, unsigned char *dst_data)
{

	// This function implements a simple pre-blur/box averaging method for
	// downsampling that gives reasonably smooth results To scale the image
	// down we will need to gather a grid of pixels of the size of the scale
	// factor in each direction and then do an averaging of the pixels.

	TArray<BoxPrecalc> vPrecalcs(height, true);
	TArray<BoxPrecalc> hPrecalcs(width, true);

	ResampleBoxPrecalc(vPrecalcs, sheight);
	ResampleBoxPrecalc(hPrecalcs, swidth);

	int averaged_pixels, averaged_alpha, src_pixel_index;
	double sum_r, sum_g, sum_b, sum_a;

	for (int y = 0; y < height; y++)         // Destination image - Y direction
	{
		// Source pixel in the Y direction
		const BoxPrecalc& vPrecalc = vPrecalcs[y];

		for (int x = 0; x < width; x++)      // Destination image - X direction
		{
			// Source pixel in the X direction
			const BoxPrecalc& hPrecalc = hPrecalcs[x];

			// Box of pixels to average
			averaged_pixels = 0;
			averaged_alpha = 0;
			sum_r = sum_g = sum_b = sum_a = 0.0;

			for (int j = vPrecalc.boxStart; j <= vPrecalc.boxEnd; ++j)
			{
				for (int i = hPrecalc.boxStart; i <= hPrecalc.boxEnd; ++i)
				{
					// Calculate the actual index in our source pixels
					src_pixel_index = j * swidth + i;

					int a = src_data[src_pixel_index * 4 + 3];
					if (a > 0)	// do not use color from fully transparent pixels
					{
						sum_r += src_data[src_pixel_index * 4 + 0];
						sum_g += src_data[src_pixel_index * 4 + 1];
						sum_b += src_data[src_pixel_index * 4 + 2];
						sum_a += a;
						averaged_pixels++;
					}
					averaged_alpha++;

				}
			}

			// Calculate the average from the sum and number of averaged pixels
			dst_data[0] = (unsigned char)xs_CRoundToInt(sum_r / averaged_pixels);
			dst_data[1] = (unsigned char)xs_CRoundToInt(sum_g / averaged_pixels);
			dst_data[2] = (unsigned char)xs_CRoundToInt(sum_b / averaged_pixels);
			dst_data[3] = (unsigned char)xs_CRoundToInt(sum_a / averaged_alpha);
			dst_data += 4;
		}
	}
}

