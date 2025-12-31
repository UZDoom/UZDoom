/*
** warptexture.cpp
**
** Texture class for warped textures
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

#include "doomtype.h"
#include "r_utility.h"
#include "r_swtexture.h"
#include "warpbuffer.h"
#include "v_video.h"

EXTERN_CVAR(Int, gl_texture_hqresizemult)
EXTERN_CVAR(Int, gl_texture_hqresizemode)
EXTERN_CVAR(Int, gl_texture_hqresize_targets)

FWarpTexture::FWarpTexture (FGameTexture *source, int warptype)
	: FSoftwareTexture (source)
{
	if (warptype == 2) SetupMultipliers(256, 128); 
	SetupMultipliers(128, 128); // [mxd]
	bWarped = warptype;
}

bool FWarpTexture::CheckModified (int style)
{
	return screen->FrameTime != GenTime[style];
}

const uint32_t *FWarpTexture::GetPixelsBgraLocked()
{
	uint64_t time = screen->FrameTime;
	uint64_t resizeMult = gl_texture_hqresizemult;

	if (time != GenTime[2])
	{
		if (gl_texture_hqresizemode == 0 || gl_texture_hqresizemult < 1 || !(gl_texture_hqresize_targets & 1))
			resizeMult = 1;

		auto otherpix = FSoftwareTexture::GetPixelsBgraLocked();
		WarpedPixelsRgba.Resize(unsigned(GetWidth() * GetHeight() * resizeMult * resizeMult * 4 / 3 + 1));
		WarpBuffer(WarpedPixelsRgba.Data(), otherpix, int(GetWidth() * resizeMult), int(GetHeight() * resizeMult), WidthOffsetMultiplier, HeightOffsetMultiplier, time, mTexture->GetShaderSpeed(), bWarped);
		GenerateBgraMipmapsFast();
		FreeAllSpans();
		GenTime[2] = time;
	}
	return WarpedPixelsRgba.Data();
}


const uint8_t *FWarpTexture::GetPixelsLocked(int index)
{
	uint64_t time = screen->FrameTime;
	uint64_t resizeMult = gl_texture_hqresizemult;

	if (time != GenTime[index])
	{
		if (gl_texture_hqresizemode == 0 || gl_texture_hqresizemult < 1 || !(gl_texture_hqresize_targets & 1))
			resizeMult = 1;

		const uint8_t *otherpix = FSoftwareTexture::GetPixelsLocked(index);
		WarpedPixels[index].Resize(unsigned(GetWidth() * GetHeight() * resizeMult * resizeMult));
		WarpBuffer(WarpedPixels[index].Data(), otherpix, int(GetWidth() * resizeMult), int(GetHeight() * resizeMult), WidthOffsetMultiplier, HeightOffsetMultiplier, time, mTexture->GetShaderSpeed(), bWarped);
		FreeAllSpans();
		GenTime[index] = time;
	}
	return WarpedPixels[index].Data();
}

// [mxd] Non power of 2 textures need different offset multipliers, otherwise warp animation won't sync across texture
void FWarpTexture::SetupMultipliers (int width, int height)
{
	WidthOffsetMultiplier = width;
	HeightOffsetMultiplier = height;
	int widthpo2 = NextPo2(GetWidth());
	int heightpo2 = NextPo2(GetHeight());
	if(widthpo2 != GetWidth()) WidthOffsetMultiplier = (int)(WidthOffsetMultiplier * ((float)widthpo2 / GetWidth()));
	if(heightpo2 != GetHeight()) HeightOffsetMultiplier = (int)(HeightOffsetMultiplier * ((float)heightpo2 / GetHeight()));
}

int FWarpTexture::NextPo2 (int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return ++v;
}

void FWarpTexture::GenerateBgraMipmapsFast()
{
	uint32_t *src = WarpedPixelsRgba.Data();
	uint32_t *dest = src + GetPhysicalWidth() * GetPhysicalHeight();
	int levels = MipmapLevels();
	for (int i = 1; i < levels; i++)
	{
		int srcw = max(GetPhysicalWidth() >> (i - 1), 1);
		int srch = max(GetPhysicalHeight() >> (i - 1), 1);
		int w = max(GetPhysicalWidth() >> i, 1);
		int h = max(GetPhysicalHeight() >> i, 1);

		for (int x = 0; x < w; x++)
		{
			int sx0 = x * 2;
			int sx1 = min((x + 1) * 2, srcw - 1);

			for (int y = 0; y < h; y++)
			{
				int sy0 = y * 2;
				int sy1 = min((y + 1) * 2, srch - 1);

				uint32_t src00 = src[sy0 + sx0 * srch];
				uint32_t src01 = src[sy1 + sx0 * srch];
				uint32_t src10 = src[sy0 + sx1 * srch];
				uint32_t src11 = src[sy1 + sx1 * srch];

				uint32_t alpha = (APART(src00) + APART(src01) + APART(src10) + APART(src11) + 2) / 4;
				uint32_t red = (RPART(src00) + RPART(src01) + RPART(src10) + RPART(src11) + 2) / 4;
				uint32_t green = (GPART(src00) + GPART(src01) + GPART(src10) + GPART(src11) + 2) / 4;
				uint32_t blue = (BPART(src00) + BPART(src01) + BPART(src10) + BPART(src11) + 2) / 4;

				dest[y + x * h] = (alpha << 24) | (red << 16) | (green << 8) | blue;
			}
		}

		src = dest;
		dest += w * h;
	}
}
