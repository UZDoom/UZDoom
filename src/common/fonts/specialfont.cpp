/*
** specialfont.cpp
**
** Font management
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2019 Christoph Oelckers
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

#include "v_font.h"
#include "textures.h"
#include "image.h"
#include "fontchars.h"
#include "texturemanager.h"
#include "i_interface.h"

#include "fontinternals.h"

// Essentially a normal multilump font but with an explicit list of character patches
class FSpecialFont : public FFont
{
public:
	FSpecialFont (const char *name, int first, int count, FGameTexture **lumplist, const bool *notranslate, int lump, bool donttranslate);

	void LoadTranslations();

protected:
	bool notranslate[256];
};


//==========================================================================
//
// FSpecialFont :: FSpecialFont
//
//==========================================================================

FSpecialFont::FSpecialFont (const char *name, int first, int count, FGameTexture **lumplist, const bool *notranslate, int lump, bool donttranslate) 
	: FFont(lump)
{
	int i;
	TArray<FGameTexture *> charlumps(count, true);
	int maxyoffs;
	FGameTexture *pic;

	memcpy(this->notranslate, notranslate, 256*sizeof(bool));

	noTranslate = donttranslate;
	FontName = name;
	Chars.Resize(count);
	FirstChar = first;
	LastChar = first + count - 1;
	FontHeight = 0;
	GlobalKerning = false;

	maxyoffs = 0;

	for (i = 0; i < count; i++)
	{
		pic = charlumps[i] = lumplist[i];
		if (pic != nullptr)
		{
			int height = (int)pic->GetDisplayHeight();
			int yoffs = (int)pic->GetDisplayTopOffset();

			if (yoffs > maxyoffs)
			{
				maxyoffs = yoffs;
			}
			height += abs (yoffs);
			if (height > FontHeight)
			{
				FontHeight = height;
			}
		}

		if (charlumps[i] != nullptr)
		{
			auto charpic = charlumps[i];
			Chars[i].OriginalPic = MakeGameTexture(charpic->GetTexture(), nullptr, ETextureType::FontChar);
			Chars[i].OriginalPic->CopySize(charpic, true);
			TexMan.AddGameTexture(Chars[i].OriginalPic);
			Chars[i].XMove = (int)Chars[i].OriginalPic->GetDisplayWidth();
			if (sysCallbacks.FontCharCreated) sysCallbacks.FontCharCreated(charpic, Chars[i].OriginalPic);
		}
		else
		{
			Chars[i].OriginalPic = nullptr;
			Chars[i].XMove = INT_MIN;
		}
	}

	// Special fonts normally don't have all characters so be careful here!
	if ('N'-first >= 0 && 'N'-first < count && Chars['N' - first].OriginalPic != nullptr)
	{
		SpaceWidth = (Chars['N' - first].XMove + 1) / 2;
	}
	else
	{
		SpaceWidth = 4;
	}

	FixXMoves();
}

//==========================================================================
//
// FSpecialFont :: LoadTranslations
//
//==========================================================================

void FSpecialFont::LoadTranslations()
{
	FFont::LoadTranslations();

	bool empty = true;
	for (auto& c : Chars)
	{
		if (c.OriginalPic != nullptr)
		{
			empty = false;
			break;
		}
	}
	if (empty) return; // Font has no characters.

	bool needsnotrans = false;
	// exclude the non-translated colors from the translation calculation
	for (int i = 0; i < 256; i++)
		if (notranslate[i])
		{
			needsnotrans = true;
			break;
		}

	// If we have no non-translateable colors, we can use the base data as-is.
	if (!needsnotrans)
	{
		return;
	}

	// we only need to add special handling if there's colors that should not be translated.
	// Obviously 'notranslate' should only be used on data that uses the base palette, otherwise results are undefined!
	for (auto &trans : Translations)
	{
		if (!IsLuminosityTranslation(trans)) continue; // this should only happen for CR_UNTRANSLATED.

		FRemapTable remap(256);
		remap.ForFont = true;

		uint8_t workpal[1024];
		for (int i = 0; i < 256; i++)
		{
			workpal[i * 4 + 0] = GPalette.BaseColors[i].b;
			workpal[i * 4 + 1] = GPalette.BaseColors[i].g;
			workpal[i * 4 + 2] = GPalette.BaseColors[i].r;
			workpal[i * 4 + 3] = GPalette.BaseColors[i].a;
		}
		V_ApplyLuminosityTranslation(LuminosityTranslationDesc::fromID(trans), workpal, 256);
		for (int i = 0; i < 256; i++)
		{
			if (!notranslate[i])
			{
				remap.Palette[i] = PalEntry(workpal[i * 4 + 3], workpal[i * 4 + 2], workpal[i * 4 + 1], workpal[i * 4 + 0]);
				remap.Remap[i] = ColorMatcher.Pick(remap.Palette[i]);
			}
			else
			{
				remap.Palette[i] = GPalette.BaseColors[i];
				remap.Remap[i] = i;
			}
		}
		trans = GPalette.StoreTranslation(TRANSLATION_Internal, &remap);
	}
}

FFont *CreateSpecialFont (const char *name, int first, int count, FGameTexture **lumplist, const bool *notranslate, int lump, bool donttranslate) 
{
	return new FSpecialFont(name, first, count, lumplist, notranslate, lump, donttranslate);
}
