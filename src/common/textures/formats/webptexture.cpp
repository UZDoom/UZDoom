/*
** webptexture.cpp
**
** Texture class for WebP images.
**
**---------------------------------------------------------------------------
**
** Copyright 2023 Cacodemon345
** Copyright 2023-2025 GZDoom Maintainers and Contributors
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
#include "webp/decode.h"
#include "webp/mux.h"

#include "files.h"
#include "filesystem.h"
#include "bitmap.h"
#include "imagehelpers.h"
#include "image.h"
#include "printf.h"

class FWebPTexture : public FImageSource
{

public:
	FWebPTexture(int lumpnum, int w, int h, int xoff, int yoff);
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override;
};


FImageSource *WebPImage_TryCreate(FileReader &file, int lumpnum)
{
	int width = 0, height = 0;
	int xoff = 0, yoff = 0;
	file.Seek(0, FileReader::SeekSet);

	uint8_t header[12];
	if (file.Read(header, 12) != 12) return nullptr;
	if (memcmp(header, "RIFF", 4) || memcmp(header + 8, "WEBP", 4)) return nullptr;

	file.Seek(0, FileReader::SeekSet);
	auto bytes = file.Read();

	if (WebPGetInfo(bytes.bytes(), bytes.size(), &width, &height))
	{
		WebPData data{ bytes.bytes(), bytes.size() };
		WebPData chunk_data;
		auto mux = WebPMuxCreate(&data, 0);
		if (mux)
		{
			const char fourcc[4] = { 'g', 'r', 'A', 'b' };
			if (WebPMuxGetChunk(mux, fourcc, &chunk_data) == WEBP_MUX_OK && chunk_data.size >= 4)
			{
				xoff = chunk_data.bytes[0] | (chunk_data.bytes[1] << 8);
				yoff = chunk_data.bytes[2] | (chunk_data.bytes[3] << 8);
			}
			WebPMuxDelete(mux);
		}
		return new FWebPTexture(lumpnum, width, height, xoff, yoff);
	}
	return nullptr;
}

FWebPTexture::FWebPTexture(int lumpnum, int w, int h, int xoff, int yoff)
	: FImageSource(lumpnum)
{
	Width = w;
	Height = h;
	LeftOffset = xoff;
	TopOffset = yoff;
}

PalettedPixels FWebPTexture::CreatePalettedPixels(int conversion, int frame)
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

int FWebPTexture::CopyPixels(FBitmap *bmp, int conversion, int frame)
{
	WebPDecoderConfig config;
	auto bytes = fileSystem.ReadFile(SourceLump);

	if (WebPInitDecoderConfig(&config) == false)
		return 0;

	config.options.no_fancy_upsampling = 0;
	config.output.colorspace = MODE_BGRA;
	config.output.u.RGBA.rgba = (uint8_t*)bmp->GetPixels();
	config.output.u.RGBA.size = bmp->GetBufferSize();
	config.output.u.RGBA.stride = bmp->GetPitch();
	config.output.is_external_memory = 1;

	(void)WebPDecode(bytes.bytes(), bytes.size(), &config);

	return 0;
}
