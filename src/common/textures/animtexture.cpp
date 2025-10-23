/*
** animtexture.cpp
**
**---------------------------------------------------------------------------
** Copyright 2020 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
**
*/
#include "animtexture.h"
#include "bitmap.h"
#include "texturemanager.h"
#include "engineerrors.h"

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
	Image.Resize(width * height * 4 * 2);
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

void AnimTexture::ClearFrame()
{
	auto dpix = Image.Data();
	for (int i = 0; i < Width * Height * 2; i++)
	{
		dpix[0] = 0;
		dpix[1] = 0;
		dpix[2] = 0;
		dpix[3] = 255;

		dpix += 4;
	}
}

void AnimTexture::SetFrame(const uint8_t* Palette, const void* data_)
{
	if (data_)
	{
		active ^= 1;
		auto dpix = Image.Data() + (active ? (Image.Size() / 2) : 0);

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
	return FBitmap(Image.Data() + (active ? (Image.Size() / 2) : 0), Width * 4, Width, Height);
}

//==========================================================================
//
//
//
//==========================================================================

AnimTextures::AnimTextures()
{
	active = 1;
	tex = TexMan.FindGameTexture("AnimTextureFrame1", ETextureType::Override);
}

AnimTextures::~AnimTextures()
{
	ClearFrame();
	Clean();
}

void AnimTextures::SetTarget(FTextureID first)
{
	Clean();

	active = 1;
	tex = TexMan.GetGameTexture(first);

	if (!tex)
		I_Error("AnimTexture not found.");

	if (tex->GetTexture()->IsAnimTex() == false)
	{
		I_Error("Invalid textures for AnimTextures specified.");
	}
}

void AnimTextures::Clean(bool clearframe)
{
	if (tex)
	{
		if (clearframe)
			ClearFrame();
		tex->CleanHardwareData(true);
	}
	tex = nullptr;
}

void AnimTextures::SetSize(int format, int width, int height)
{
	static_cast<AnimTexture*>(tex->GetTexture())->SetFrameSize(format, width, height);
	tex->SetSize(width, height);
	tex->CleanHardwareData();
}

void AnimTextures::ClearFrame()
{
	if (!tex)
		return;
	static_cast<AnimTexture*>(tex->GetTexture())->ClearFrame();
	tex->CleanHardwareData();
}

void AnimTextures::SetFrame(const uint8_t* palette, const void* data)
{
	static_cast<AnimTexture*>(tex->GetTexture())->SetFrame(palette, data);
	tex->CleanHardwareData();
}
