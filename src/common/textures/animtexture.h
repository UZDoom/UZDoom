/*
** animtexture.h
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

#pragma once

#include "textures.h"


class AnimTexture : public FTexture
{
	TArray<uint8_t> Image;
	int pixelformat;
public:
	enum
	{
		Paletted = 0,
		RGB = 1,
		YUV = 2,
		VPX = 3
	};
	AnimTexture() = default;
	void SetFrameSize(int format, int width, int height);
	void SetFrame(const uint8_t* palette, const void* data);
	virtual FBitmap GetBgraBitmap(const PalEntry* remap, int* trans) override;
};

class AnimTextures
{
	int active;
	FGameTexture* tex[2];

public:
	AnimTextures();
	~AnimTextures();
	void Clean();
	void SetSize(int format, int width, int height);
	void SetFrame(const uint8_t* palette, const void* data);
	FGameTexture* GetFrame()
	{
		return tex[active];
	}

	FTextureID GetFrameID()
	{
		return tex[active]->GetID();
	}
};
