/*
** animtexture.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2020 Christoph Oelckers
** Copyright 2020-2025 GZDoom Maintainers and Contributors
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

#include "animtexture.h"
#include "bitmap.h"
#include "texturemanager.h"

#include "vpx/vpx_image.h"


//==========================================================================
//
//
//
//==========================================================================

void AnimTexture::SetFrameSize(int  format, int width, int height)
{
	pixelformat = format;
	FTexture::SetSize(width, height);
	Image.Resize(width * height * 4);
	memset(Image.Data(), 0, Image.Size());
}

//TODO optimize
static inline void YUVtoRGB(uint8_t yi, uint8_t ui, uint8_t vi, uint8_t * rgb)
{
	float Y = yi * (1 / 255.f);
	float U = ui * (1 / 255.f) - 0.5f;
	float V = vi * (1 / 255.f) - 0.5f;
	Y = 1.1643f * (Y - 0.0625f);
	float r = Y + 1.5958f * V;
	float g = Y - 0.39173f * U - 0.81290f * V;
	float b = Y + 2.017f * U;
	(rgb)[2] = (uint8_t)(clamp(r, 0.f, 1.f) * 255);
	(rgb)[1] = (uint8_t)(clamp(g, 0.f, 1.f) * 255);
	(rgb)[0] = (uint8_t)(clamp(b, 0.f, 1.f) * 255);
	(rgb)[3] = 255;
}

void AnimTexture::SetFrame(const uint8_t* Palette, const void* data_)
{
	if (data_)
	{
		auto dpix = Image.Data();

		if (pixelformat == YUV)
		{
			const uint8_t * spix = reinterpret_cast<const uint8_t *>(data_);

			for (int i = 0; i < Width * Height; i++)
			{
				YUVtoRGB(spix[0], spix[1], spix[2], dpix);

				spix += 4;
				dpix += 4;
			}
		}
		else if(pixelformat == VPX)
		{
			const vpx_image_t *img = reinterpret_cast<const vpx_image_t *>(data_);
			
			uint8_t const* const yplane = img->planes[VPX_PLANE_Y];
			uint8_t const* const uplane = img->planes[VPX_PLANE_U];
			uint8_t const* const vplane = img->planes[VPX_PLANE_V];

			const int ystride = img->stride[VPX_PLANE_Y];
			const int ustride = img->stride[VPX_PLANE_U];
			const int vstride = img->stride[VPX_PLANE_V];

			if(img->fmt == VPX_IMG_FMT_I420)
			{
				for (unsigned int y = 0; y < Height; y++)
				{
					for (unsigned int x = 0; x < Width; x++)
					{
						YUVtoRGB(
								yplane[ystride * y + x],
								uplane[ustride * (y >> 1) + (x >> 1)],
								vplane[vstride * (y >> 1) + (x >> 1)],
								dpix
						);

						dpix += 4;
					}
				}
			}
			else if(img->fmt == VPX_IMG_FMT_I444)
			{
				for (unsigned int y = 0; y < Height; y++)
				{
					for (unsigned int x = 0; x < Width; x++)
					{
						YUVtoRGB(
							yplane[ystride * y + x],
							uplane[ustride * y + x],
							vplane[vstride * y + x],
							dpix
						);
						dpix += 4;
					}
				}
			}
			else if(img->fmt == VPX_IMG_FMT_I422)
			{ // 422 and 440 untested
				for (unsigned int y = 0; y < Height; y++)
				{
					for (unsigned int x = 0; x < Width; x++)
					{
						YUVtoRGB(
							yplane[ystride * y + x],
							uplane[ustride * y + (x >> 1)],
							vplane[vstride * y + (x >> 1)],
							dpix
						);
						dpix += 4;
					}
				}
			}
			else if(img->fmt == VPX_IMG_FMT_I440)
			{
				for (unsigned int y = 0; y < Height; y++)
				{
					for (unsigned int x = 0; x < Width; x++)
					{
						YUVtoRGB(
							yplane[ystride * y + x],
							uplane[ustride * (y >> 1) + x],
							vplane[vstride * (y >> 1) + x],
							dpix
						);
						dpix += 4;
					}
				}
			}
		}
		else if(pixelformat == RGB)
		{
			const uint8_t *img = reinterpret_cast<const uint8_t *>(data_);

			for (int i = 0; i < Width * Height; i++)
			{
				dpix[0] = img[2];
				dpix[1] = img[1];
				dpix[2] = img[0];
				dpix[3] = 255;

				dpix += 4;
			}
		}
		else if (pixelformat == Paletted)
		{
			assert(Palette);
			const uint8_t *img = reinterpret_cast<const uint8_t *>(data_);

			for (int i = 0; i < Width * Height; i++)
			{
				int index = img[i];
				dpix[0] = Palette[index * 3 + 2];
				dpix[1] = Palette[index * 3 + 1];
				dpix[2] = Palette[index * 3];
				dpix[3] = 255;

				dpix += 4;
			}
		}
	}
}

//===========================================================================
//
// FPNGTexture::CopyPixels
//
//===========================================================================

FBitmap AnimTexture::GetBgraBitmap(const PalEntry* remap, int* trans)
{
	return FBitmap(Image.Data(), Width * 4, Width, Height);
}

//==========================================================================
//
//
//
//==========================================================================

AnimTextures::AnimTextures()
{
	active = 1;
	tex[0] = TexMan.FindGameTexture("AnimTextureFrame1", ETextureType::Override);
	tex[1] = TexMan.FindGameTexture("AnimTextureFrame2", ETextureType::Override);
}

AnimTextures::~AnimTextures()
{
	Clean();
}

void AnimTextures::Clean()
{
	if (tex[0]) tex[0]->CleanHardwareData(true);
	if (tex[1]) tex[1]->CleanHardwareData(true);
	tex[0] = tex[1] = nullptr;
}

void AnimTextures::SetSize(int format, int width, int height)
{
	static_cast<AnimTexture*>(tex[0]->GetTexture())->SetFrameSize(format, width, height);
	static_cast<AnimTexture*>(tex[1]->GetTexture())->SetFrameSize(format, width, height);
	tex[0]->SetSize(width, height);
	tex[1]->SetSize(width, height);
	tex[0]->CleanHardwareData();
	tex[1]->CleanHardwareData();
}

void AnimTextures::SetFrame(const uint8_t* palette, const void* data)
{
	active ^= 1;
	static_cast<AnimTexture*>(tex[active]->GetTexture())->SetFrame(palette, data);
	tex[active]->CleanHardwareData();
}
