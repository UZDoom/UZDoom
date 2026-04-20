/*
** firetexture.h
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

#pragma once

#include "textures.h"


class FireTexture : public FTexture
{
	TArray<uint8_t> Image;
	TArray<PalEntry> Palette;

public:
	FireTexture();
	void Reset();
	void SetPalette(TArray<PalEntry>& colors);
	void Update();
	virtual FBitmap GetBgraBitmap(const PalEntry* remap, int* trans) override;
	virtual TArray<uint8_t> Get8BitPixels(bool alphatex) override;
};
