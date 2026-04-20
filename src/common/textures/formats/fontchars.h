/*
** fontchars.h
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

// This is a font character that reads RLE compressed data.
class FFontChar2 : public FImageSource
{
public:
	FFontChar2 (int sourcelump, int sourcepos, int width, int height, int leftofs=0, int topofs=0);

	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
	int CopyPixels(FBitmap* bmp, int conversion, int frame = 0);

	void SetSourceRemap(const PalEntry* sourceremap)
	{
		SourceRemap = sourceremap;
	}

protected:
	int SourceLump;
	int SourcePos;
	const PalEntry *SourceRemap;
};
