/*
** font.cpp
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

// HEADER FILES ------------------------------------------------------------

#include <cwctype>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "m_swap.h"
#include "v_font.h"
#include "printf.h"
#include "textures.h"
#include "filesystem.h"
#include "cmdlib.h"
#include "sc_man.h"
#include "gstrings.h"
#include "image.h"
#include "utf8.h"
#include "myiswalpha.h"
#include "fontchars.h"
#include "multipatchtexture.h"
#include "texturemanager.h"
#include "i_interface.h"

#include "fontinternals.h"
#include "Trex/Atlas.hpp"
#include "texturemanager.h"
#include "Trex/TextShaper.hpp"
#include "c_cvars.h"
#include "simdutf.h"
#include <string>
#include "menu.h"
#include "vm.h"
#include "c_dispatch.h"

TArray<FBitmap> sheetBitmaps;

CVAR(String, fontoverride_Fallback, "HACK-BOL", CVAR_ARCHIVE);
CVAR(String, fontoverride_FallbackJP, "IBMPLEXJ", CVAR_ARCHIVE);
CVAR(String, fontoverride_FallbackKR, "IBMPLEXK", CVAR_ARCHIVE);
CVAR(String, fontoverride_FallbackCYR, "HACK-BOL", CVAR_ARCHIVE);
CVAR(String, fontoverride_FallbackSC, "ZCOOLQin", CVAR_ARCHIVE);
CVAR(String, fontoverride_FallbackTH, "CHAKRA_P", CVAR_ARCHIVE);

void SetFontChoice_SmallText(const char *newFont);
CUSTOM_CVAR(String, fontchoice_smalltext, "FO_DEFAULT", CVAR_ARCHIVE)
{
	SetFontChoice_SmallText(self);
}

void SetFontChoice_Title(const char *newFont);
CUSTOM_CVAR(String, fontchoice_title, "FO_DEFAULT", CVAR_ARCHIVE)
{
	SetFontChoice_Title(self);
}

void SetFontChoice_Description(const char *newFont);
CUSTOM_CVAR(String, fontchoice_description, "FO_DEFAULT", CVAR_ARCHIVE)
{
	SetFontChoice_Description(self);
}

void SetFontChoice_Console(const char *newFont);
CUSTOM_CVAR(String, fontchoice_console, "FO_DEFAULT", CVAR_ARCHIVE)
{
	SetFontChoice_Console(self);
}

void SetFontChoice_BigText(const char *newFont);
CUSTOM_CVAR(String, fontchoice_bigtext, "FO_DEFAULT", CVAR_ARCHIVE)
{
	SetFontChoice_BigText(self);
}

CVAR(String, fontchoice_smalltextJP, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_bigtextJP, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_titleJP, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_descriptionJP, "FO_DEFAULT", CVAR_ARCHIVE);

CVAR(String, fontchoice_smalltextKO, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_bigtextKO, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_titleKO, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_descriptionKO, "FO_DEFAULT", CVAR_ARCHIVE);

CVAR(String, fontchoice_smalltextCYR, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_bigtextCYR, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_titleCYR, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_descriptionCYR, "FO_DEFAULT", CVAR_ARCHIVE);

CVAR(String, fontchoice_smalltextSC, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_bigtextSC, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_titleSC, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_descriptionSC, "FO_DEFAULT", CVAR_ARCHIVE);

CVAR(String, fontchoice_smalltextTH, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_bigtextTH, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_titleTH, "FO_DEFAULT", CVAR_ARCHIVE);
CVAR(String, fontchoice_descriptionTH, "FO_DEFAULT", CVAR_ARCHIVE);

void FindOrCreateFontChoiceCVAR(FString varName)
{
	FBaseCVar *var            = nullptr;
	var                       = FindCVar(varName.GetChars(), nullptr);
	if (var == nullptr)
	{
		var = new FStringCVar(varName.GetChars(), nullptr, CVAR_AUTO | CVAR_ARCHIVE | cvar_defflags);
		var->CmdSet("FO-DEFAULT");
	}
}

void FFont::MakeFontChoiceCVARs()
{
	FFont::UpdateAdvFontMappingTables();
	auto&langMap = GStrings.GetLangMap();
	for (const auto& [l, id] : langMap)
	{
		if (id.name.IsValidName())
		{
			FString      langNameString = FString(id.name.GetChars());
			FindOrCreateFontChoiceCVAR("fontfallback_" + langNameString);	
			FindOrCreateFontChoiceCVAR("fontchoice_smalltext_" + langNameString);	
			FindOrCreateFontChoiceCVAR("fontchoice_bigtext_" + langNameString);	
			FindOrCreateFontChoiceCVAR("fontchoice_title_" + langNameString);	
			FindOrCreateFontChoiceCVAR("fontchoice_description_" + langNameString);

			auto staticFonts = FFont::GetRemappableFonts();
			for (auto f : staticFonts)
			{
				FString fontName = f->GetName().GetChars();
				FindOrCreateFontChoiceCVAR("fontchoice_" + fontName + "_" + langNameString);
			}
		}
	}
}

//==========================================================================
//
// FFont :: FFont
//
// Loads a multi-texture font.
//
//==========================================================================

FFont::FFont (const char *name, const char *nametemplate, const char *filetemplate, int lfirst, int lcount, int start, int fdlump, int spacewidth, bool notranslate, bool iwadonly, bool doomtemplate, GlyphSet *baseGlyphs)
	: FFont(fdlump, name)
{
	int i;
	FTextureID lump;
	char buffer[12];
	DVector2 Scale = { 1, 1 };

	noTranslate = notranslate;
	GlobalKerning = false;
	SpaceWidth = 0;
	FontHeight = 0;
	int FixedWidth = 0;

	TMap<int, FGameTexture*> charMap;
	int minchar = INT_MAX;
	int maxchar = INT_MIN;

	// Read the font's configuration.
	// This will not be done for the default fonts, because they are not atomic and the default content does not need it.

	std::vector<FileSys::FolderEntry> folderdata;
	if (filetemplate != nullptr)
	{
		FStringf path("fonts/%s/", filetemplate);
		// If a name template is given, collect data from all resource files.
		// For anything else, each folder is being treated as an atomic, self-contained unit and mixing from different glyph sets is blocked.
		fileSystem.GetFilesInFolder(path.GetChars(), folderdata, nametemplate == nullptr);

		//if (nametemplate == nullptr)
		{
			FStringf infpath("fonts/%s/font.inf", filetemplate);

			size_t index;
			for(index = 0; index < folderdata.size(); index++)
			{
				if (infpath.CompareNoCase(folderdata[index].name) == 0) break;
			}

			if (index < folderdata.size())
			{
				FScanner sc;
				sc.OpenLumpNum(folderdata[index].lumpnum);
				while (sc.GetToken())
				{
					sc.TokenMustBe(TK_Identifier);
					if (sc.Compare("Kerning"))
					{
						sc.MustGetValue(false);
						GlobalKerning = sc.Number;
					}
					else if (sc.Compare("Altfont"))
					{
						sc.MustGetString();
						AltFontName = sc.String;
					}
					else if (sc.Compare("Scale"))
					{
						sc.MustGetValue(true);
						Scale.Y = Scale.X = sc.Float;
						if (sc.CheckToken(','))
						{
							sc.MustGetValue(true);
							Scale.Y = sc.Float;
						}
					}
					else if (sc.Compare("SpaceWidth"))
					{
						sc.MustGetValue(false);
						SpaceWidth = sc.Number;
					}
					else if (sc.Compare("FontHeight"))
					{
						sc.MustGetValue(false);
						FontHeight = sc.Number;
					}
					else if (sc.Compare("CellSize"))
					{
						sc.MustGetValue(false);
						FixedWidth = sc.Number;
						sc.MustGetToken(',');
						sc.MustGetValue(false);
						FontHeight = sc.Number;
					}
					else if (sc.Compare("minluminosity"))
					{
						sc.MustGetValue(false);
						MinLum = (int16_t)clamp(sc.Number, 0, 255);
					}
					else if (sc.Compare("maxluminosity"))
					{
						sc.MustGetValue(false);
						MaxLum = (int16_t)clamp(sc.Number, 0, 255);
					}
					else if (sc.Compare("Translationtype"))
					{
						sc.MustGetToken(TK_Identifier);
						if (sc.Compare("console"))
						{
							TranslationType = 1;
						}
						else if (sc.Compare("standard"))
						{
							TranslationType = 0;
						}
						else
						{
							sc.ScriptError("Unknown translation type %s", sc.String);
						}
					}
					else if (sc.Compare("lowercaselatinonly"))
					{
						lowercaselatinonly = true;
					}

				}
			}
		}
	}

	if (FixedWidth > 0)
	{
		ReadSheetFont(folderdata, FixedWidth, FontHeight, Scale);
		Type = Folder;
	}
	else
	{
		if (baseGlyphs)
		{
			// First insert everything from the given glyph set.
			GlyphSet::Iterator it(*baseGlyphs);
			GlyphSet::Pair* pair;
			while (it.NextPair(pair))
			{
				if (pair->Value && pair->Value->GetTexelWidth() > 0 && pair->Value->GetTexelHeight() > 0)
				{
					auto position = pair->Key;
					if (position < minchar) minchar = position;
					if (position > maxchar) maxchar = position;
					charMap.Insert(position, pair->Value);
				}
			}
		}
		if (nametemplate != nullptr)
		{
			if (!iwadonly)
			{
				for (i = 0; i < lcount; i++)
				{
					int position = lfirst + i;
					mysnprintf(buffer, countof(buffer), nametemplate, i + start);

					lump = TexMan.CheckForTexture(buffer, ETextureType::MiscPatch);
					if (doomtemplate && lump.isValid() && i + start == 121)
					{ // HACKHACK: Don't load STCFN121 in doom(2), because
					  // it's not really a lower-case 'y' but a '|'.
					  // Because a lot of wads with their own font seem to foolishly
					  // copy STCFN121 and make it a '|' themselves, wads must
					  // provide STCFN120 (x) and STCFN122 (z) for STCFN121 to load as a 'y'.
						FStringf c120(nametemplate, 120);
						FStringf c122(nametemplate, 122);
						if (!TexMan.CheckForTexture(c120.GetChars(), ETextureType::MiscPatch).isValid() ||
							!TexMan.CheckForTexture(c122.GetChars(), ETextureType::MiscPatch).isValid())
						{
							// insert the incorrectly named '|' graphic in its correct position.
							position = 124;
						}
					}
					if (lump.isValid())
					{
						Type = Multilump;
						if (position < minchar) minchar = position;
						if (position > maxchar) maxchar = position;
						charMap.Insert(position, TexMan.GetGameTexture(lump));
					}
				}
			}
			else
			{
				FGameTexture *texs[256] = {};
				if (lcount > 256 - start) lcount = 256 - start;
				for (i = 0; i < lcount; i++)
				{
					TArray<FTextureID> array;
					mysnprintf(buffer, countof(buffer), nametemplate, i + start);

					TexMan.ListTextures(buffer, array, true);
					for (auto entry : array)
					{
						auto tex = TexMan.GetGameTexture(entry, false);
						if (tex && !tex->isUserContent() && tex->GetUseType() == ETextureType::MiscPatch)
						{
							texs[i] = tex;
						}
					}
				}
				if (doomtemplate)
				{
					// Handle the misplaced '|'.
					if (texs[121 - '!'] && !texs[120 - '!'] && !texs[122 - '!'] && !texs[124 - '!'])
					{
						texs[124 - '!'] = texs[121 - '!'];
						texs[121 - '!'] = nullptr;
					}
				}

				for (i = 0; i < lcount; i++)
				{
					if (texs[i])
					{
						int position = lfirst + i;
						Type = Multilump;
						if (position < minchar) minchar = position;
						if (position > maxchar) maxchar = position;
						charMap.Insert(position, texs[i]);
					}
				}
			}
		}
		if (folderdata.size() > 0)
		{
			// all valid lumps must be named with a hex number that represents its Unicode character index.
			for (auto &entry : folderdata)
			{
				char *endp;
				auto base = ExtractFileBase(entry.name);
				auto position = strtoll(base.GetChars(), &endp, 16);
				if ((*endp == 0 || (*endp == '.' && position >= '!' && position < 0xffff)))
				{
					auto texlump = TexMan.CheckForTexture(entry.name, ETextureType::MiscPatch);
					if (texlump.isValid())
					{
						if ((int)position < minchar) minchar = (int)position;
						if ((int)position > maxchar) maxchar = (int)position;
						auto tex = TexMan.GetGameTexture(texlump);
						tex->SetScale((float)Scale.X, (float)Scale.Y);
						charMap.Insert((int)position, tex);
						Type = Folder;
					}
				}
			}
		}
		FirstChar = minchar;
		LastChar = maxchar;
		auto count = maxchar - minchar + 1;
		Chars.Resize(count);
		int fontheight = 0;

		for (i = 0; i < count; i++)
		{
			auto charlump = charMap.CheckKey(FirstChar + i);
			if (charlump != nullptr)
			{
				auto pic = *charlump;
				if (pic != nullptr)
				{
					double fheight = pic->GetDisplayHeight();
					double yoffs = pic->GetDisplayTopOffset();

					int height = int(fheight + abs(yoffs) + 0.5);
					if (height > fontheight)
					{
						fontheight = height;
					}
				}

				auto orig = pic->GetTexture();
				auto tex = MakeGameTexture(orig, nullptr, ETextureType::FontChar);
				tex->CopySize(pic, true);
				TexMan.AddGameTexture(tex);
				Chars[i].OriginalPic = tex;

				if (sysCallbacks.FontCharCreated) sysCallbacks.FontCharCreated(pic, Chars[i].OriginalPic);

				Chars[i].XMove = (int)Chars[i].OriginalPic->GetDisplayWidth();
			}
			else
			{
				Chars[i].OriginalPic = nullptr;
				Chars[i].XMove = INT_MIN;
			}
		}

		if (SpaceWidth == 0) // An explicit override from the .inf file must always take precedence
		{
			if (spacewidth != -1)
			{
				SpaceWidth = spacewidth;
			}
			else if ('N' - FirstChar >= 0 && 'N' - FirstChar < count && Chars['N' - FirstChar].OriginalPic != nullptr)
			{
				SpaceWidth = (Chars['N' - FirstChar].XMove + 1) / 2;
			}
			else
			{
				SpaceWidth = 4;
			}
		}
		if (FontHeight == 0) FontHeight = fontheight;

		FixXMoves();
	}
}

class FSheetTexture : public FImageSource
{
	unsigned baseSheet;
	int X, Y;

public:

	FSheetTexture(unsigned source, int x, int y, int width, int height)
	{
		baseSheet = source;
		Width = width;
		Height = height;
		X = x;
		Y = y;
	}

	int CopyPixels(FBitmap* dest, int conversion, int frame = 0) override
	{
		auto& pic = sheetBitmaps[baseSheet];
		dest->CopyPixelDataRGB(0, 0, pic.GetPixels() + 4 * (X + pic.GetWidth() * Y), Width, Height, 4, pic.GetWidth() * 4, 0, CF_BGRA);
		return 0;
	}

};
void FFont::ReadSheetFont(std::vector<FileSys::FolderEntry> &folderdata, int width, int height, const DVector2 &Scale)
{
	TMap<int, FGameTexture*> charMap;
	int minchar = INT_MAX;
	int maxchar = INT_MIN;
	for (auto &entry : folderdata)
	{
		char *endp;
		auto base = ExtractFileBase(entry.name);
		auto position = strtoll(base.GetChars(), &endp, 16);
		if ((*endp == 0 || (*endp == '.' && position >= 0 && position < 0xffff)))	// Sheet fonts may fill in the low control chars.
		{
			auto lump = TexMan.CheckForTexture(entry.name, ETextureType::MiscPatch);
			if (lump.isValid())
			{
				auto tex = TexMan.GetGameTexture(lump);
				int numtex_x = tex->GetTexelWidth() / width;
				int numtex_y = tex->GetTexelHeight() / height;
				int maxinsheet = int(position) + numtex_x * numtex_y - 1;
				if (minchar > position) minchar = int(position);
				if (maxchar < maxinsheet) maxchar = maxinsheet;

				FBitmap* sheetimg = &sheetBitmaps[sheetBitmaps.Reserve(1)];
				sheetimg->Create(tex->GetTexelWidth(), tex->GetTexelHeight());
				tex->GetTexture()->GetImage()->CopyPixels(sheetimg, FImageSource::normal, 0);

				for (int y = 0; y < numtex_y; y++)
				{
					for (int x = 0; x < numtex_x; x++)
					{
						auto image = new FSheetTexture(sheetBitmaps.Size() - 1, x * width, y * height, width, height);
						FImageTexture *imgtex = new FImageTexture(image);
						auto gtex = MakeGameTexture(imgtex, nullptr, ETextureType::FontChar);
						gtex->SetWorldPanning(true);
						gtex->SetOffsets(0, 0, 0);
						gtex->SetOffsets(1, 0, 0);
						gtex->SetScale((float)Scale.X, (float)Scale.Y);
						TexMan.AddGameTexture(gtex);
						charMap.Insert(int(position) + x + y * numtex_x, gtex);
					}
				}
			}
		}
	}


	FirstChar = minchar;
	bool map1252 = false;
	if (minchar < 0x80 && maxchar >= 0xa0) // should be a settable option, but that'd probably cause more problems than it'd solve.
	{
		if (maxchar < 0x2122) maxchar = 0x2122;
		map1252 = true;
	}
	LastChar = maxchar;
	auto count = maxchar - minchar + 1;
	Chars.Resize(count);

	for (int i = 0; i < count; i++)
	{
		auto lump = charMap.CheckKey(FirstChar + i);
		if (lump != nullptr)
		{
			auto pic = (*lump)->GetTexture();
			Chars[i].OriginalPic = (*lump)->GetUseType() == ETextureType::FontChar? (*lump) : MakeGameTexture(pic, nullptr, ETextureType::FontChar);
			Chars[i].OriginalPic->SetUseType(ETextureType::FontChar);
			Chars[i].OriginalPic->CopySize(*lump, true);
			if (Chars[i].OriginalPic != *lump) TexMan.AddGameTexture(Chars[i].OriginalPic);
		}
		Chars[i].XMove = int(width / Scale.X);
	}

	if (map1252)
	{
		// Move the Windows-1252 characters to their proper place.
		for (int i = 0x80; i < 0xa0; i++)
		{
			if (win1252map[i - 0x80] != i && Chars[i - minchar].OriginalPic != nullptr && Chars[win1252map[i - 0x80] - minchar].OriginalPic == nullptr)
			{
				std::swap(Chars[i - minchar], Chars[win1252map[i - 0x80] - minchar]);
			}
		}
	}

	SpaceWidth = width;
}

//==========================================================================
//
// FFont :: ~FFont
//
//==========================================================================

FFont::~FFont ()
{
	FFont **prev = &FirstFont;
	FFont *font = *prev;

	while (font != nullptr && font != this)
	{
		prev = &font->Next;
		font = *prev;
	}

	if (font != nullptr)
	{
		*prev = font->Next;
	}
}

//==========================================================================
//
// FFont :: CheckCase
//
//==========================================================================

void FFont::CheckCase()
{
	int lowercount = 0, uppercount = 0;
	for (unsigned i = 0; i < Chars.Size(); i++)
	{
		unsigned chr = i + FirstChar;
		if (lowerforupper[chr] == chr && upperforlower[chr] == chr)
		{
			continue;	// not a letter;
		}
		if (myislower(chr))
		{
			if (Chars[i].OriginalPic != nullptr) lowercount++;
		}
		else
		{
			if (Chars[i].OriginalPic != nullptr) uppercount++;
		}
	}
	if (lowercount == 0) return;	// This is an uppercase-only font and we are done.

	// The ß needs special treatment because it is far more likely to be supplied lowercase only, even in an uppercase font.
	if (Chars[0xdf - FirstChar].OriginalPic != nullptr)
	{
		if (LastChar < 0x1e9e)
		{
			Chars.Resize(0x1e9f - FirstChar);
			LastChar = 0x1e9e;
		}
		if (Chars[0x1e9e - FirstChar].OriginalPic == nullptr)
		{
			std::swap(Chars[0xdf - FirstChar], Chars[0x1e9e - FirstChar]);
			lowercount--;
			uppercount++;
			if (lowercount == 0) return;
		}
	}
}

//==========================================================================
//
// FFont :: FindFont
//
// Searches for the named font in the list of loaded fonts, returning the
// font if it was found. The disk is not checked if it cannot be found.
//
//==========================================================================

FFont *FFont::FindFont (FName name)
{
	if (name == NAME_None)
	{
		return nullptr;
	}
	FFont *font = FirstFont;

	while (font != nullptr)
	{
		if (font->FontName == name) return font;
		font = font->Next;
	}
	return nullptr;
}

FFont *FindDynamicFallbackFontForLanguage(const char *lang)
{
	FString language = lang;
	if (language.CompareNoCase("ko")==0)
	{
		return V_GetFont(*fontoverride_FallbackKR);
	}
	else if (language.CompareNoCase("jp") == 0)
	{
		return V_GetFont(*fontoverride_FallbackJP);
	}
	else if (language.CompareNoCase("ru") == 0 ||
		language.CompareNoCase("sr") == 0)
	{
		return V_GetFont(*fontoverride_FallbackCYR);
	}
	else if (language.CompareNoCase("zh-Hans") == 0)
	{
		return V_GetFont(*fontoverride_FallbackSC);
	}
	else if (language.CompareNoCase("th") == 0)
	{
		return V_GetFont(*fontoverride_FallbackTH);
	}

	return V_GetFont(*fontoverride_Fallback);
}

void SetFontChoice_SmallText(const char *newFont)
{
	//
}

void SetFontChoice_Title(const char *newFont)
{
	//
}

void SetFontChoice_Description(const char *newFont)
{
	//TODO: need to refresh tooltip font
}

void SetFontChoice_Console(const char *newFont)
{
	//
}

void SetFontChoice_BigText(const char *newFont)
{
	//main menu needs to be rebuilt if this changes

}

void FFont::UpdateAdvFontMappingTables()
{
	RemappableFonts.clear();
	for (const FFont *currentFont = FFont::GetFontListHead(); currentFont; currentFont = currentFont->GetNextFont())
	{
		if (currentFont->CanBeSubstitutedWithDynamic())
		{
			RemappableFonts.push_back(currentFont);
		}
	}
}

FFont *FFont::GetDynamicSubstitutionForStaticFont(FFont *const fontToSub)
{
	if (fontToSub->CanBeSubstitutedWithDynamic())
	{
		FFont *foundDynamicRemap = nullptr;
		const FName subbedFontName = fontToSub->GetName();
		const FString subbedFontNameString = subbedFontName.GetChars();

		const FString langIDString = GStrings.GetActiveLangID().name.GetChars();
		const FString cvarName     = "fontchoice_" + subbedFontNameString + "_" + langIDString;
		if (FStringCVar *const cv = dynamic_cast<FStringCVar *>(FindCVar(cvarName.GetChars(), nullptr)))
		{
			foundDynamicRemap = V_GetFont(*cv);
		}
		
		if (foundDynamicRemap)
		{
			return foundDynamicRemap;
		}
		else
		{
			//call MenuDelegate.PickFont() in script
			//TODO: safety?
			const PClass     *const pCls = menuDelegate->GetClass();
			VMFunction *const func = PClass::FindFunction(pCls->TypeName, "PickFont");
			void       *const ret  = CallVM<void *>(func, menuDelegate, (void *)fontToSub);
			FFont* foundFont = static_cast<FFont *>(ret);
			if (foundFont->IsValidDynamicFont())
			{
				return foundFont;
			}
			else
			{
				return nullptr;
			}
		}
	}
	return nullptr;
}

FFont *FFont::GetSmallTextFont(FFont* const fallbackIfNoUserChoice)
{
	FFont  *userFont = nullptr;
	const FString lang     = GStrings.GetActiveLanguage();

	const FString langIDString = GStrings.GetActiveLangID().name.GetChars();
	const FString cvarName     = "fontchoice_smalltext_" + langIDString;
	if (FStringCVar* cv = dynamic_cast<FStringCVar*>(FindCVar(cvarName.GetChars(), nullptr)))
	{
		userFont = V_GetFont(*cv);
	}

	if (!userFont)
	{
		if (lang.CompareNoCase("jp") == 0)
		{
			userFont = V_GetFont(*fontchoice_smalltextJP);
		}
		else if (lang.CompareNoCase("ko") == 0)
		{
			userFont = V_GetFont(*fontchoice_smalltextKO);
		}
		else if (lang.CompareNoCase("ru") == 0 || lang.CompareNoCase("sr") == 0)
		{
			userFont = V_GetFont(*fontchoice_smalltextCYR);
		}
		else if (lang.CompareNoCase("zh-Hans") == 0)
		{
			userFont = V_GetFont(*fontchoice_smalltextSC);
		}
		else if (lang.CompareNoCase("th") == 0)
		{
			userFont = V_GetFont(*fontchoice_smalltextTH);
		}
		else
		{
			userFont = V_GetFont(*fontchoice_smalltext);
		}
	}

	if (userFont)
	{
		return userFont;
	}
	return fallbackIfNoUserChoice;
}

FFont *FFont::GetTitleFont(FFont *const fallbackIfNoUserChoice)
{
	FFont *userFont = nullptr;
	const FString lang = GStrings.GetActiveLanguage();

	const FString langIDString = GStrings.GetActiveLangID().name.GetChars();
	const FString cvarName     = "fontchoice_title_" + langIDString;
	if (FStringCVar *const cv = dynamic_cast<FStringCVar *>(FindCVar(cvarName.GetChars(), nullptr)))
	{
		userFont = V_GetFont(*cv);
	}

	if (!userFont)
	{
		if (lang.CompareNoCase("jp") == 0)
		{
			userFont = V_GetFont(*fontchoice_titleJP);
		}
		else if (lang.CompareNoCase("ko") == 0)
		{
			userFont = V_GetFont(*fontchoice_titleKO);
		}
		else if (lang.CompareNoCase("ru") == 0 || lang.CompareNoCase("sr") == 0)
		{
			userFont = V_GetFont(*fontchoice_titleCYR);
		}
		else if (lang.CompareNoCase("zh-Hans") == 0)
		{
			userFont = V_GetFont(*fontchoice_titleSC);
		}
		else if (lang.CompareNoCase("th") == 0)
		{
			userFont = V_GetFont(*fontchoice_titleTH);
		}
		else
		{
			userFont = V_GetFont(*fontchoice_title);
		}
	}

	if (userFont)
	{
		return userFont;
	}
	return fallbackIfNoUserChoice;
}

FFont *FFont::GetDescriptionFont(FFont *const fallbackIfNoUserChoice)
{
	FFont *userFont = nullptr;
	const FString lang     = GStrings.GetActiveLanguage();

	const FString langIDString = GStrings.GetActiveLangID().name.GetChars();
	const FString cvarName     = "fontchoice_description_" + langIDString;
	if (FStringCVar *const cv = dynamic_cast<FStringCVar *>(FindCVar(cvarName.GetChars(), nullptr)))
	{
		userFont = V_GetFont(*cv);
	}

	if (!userFont)
	{
		if (lang.CompareNoCase("jp") == 0)
		{
			userFont = V_GetFont(*fontchoice_descriptionJP);
		}
		else if (lang.CompareNoCase("ko") == 0)
		{
			userFont = V_GetFont(*fontchoice_descriptionKO);
		}
		else if (lang.CompareNoCase("ru") == 0 || lang.CompareNoCase("sr") == 0)
		{
			userFont = V_GetFont(*fontchoice_descriptionCYR);
		}
		else if (lang.CompareNoCase("zh-Hans") == 0)
		{
			userFont = V_GetFont(*fontchoice_descriptionSC);
		}
		else if (lang.CompareNoCase("th") == 0)
		{
			userFont = V_GetFont(*fontchoice_descriptionTH);
		}
		else
		{
			userFont = V_GetFont(*fontchoice_description);
		}
	}

	if (userFont)
	{
		return userFont;
	}
	return fallbackIfNoUserChoice;
}

FFont *FFont::GetConsoleFont(FFont *const fallbackIfNoUserChoice)
{
	FFont *const userFont = V_GetFont(*fontchoice_console);
	if (userFont)
	{
		return userFont;
	}
	return fallbackIfNoUserChoice;
}

FFont *FFont::GetBigTextFont(FFont *const fallbackIfNoUserChoice)
{
	FFont  *userFont = nullptr;
	const FString lang     = GStrings.GetActiveLanguage();

	const FString langIDString = GStrings.GetActiveLangID().name.GetChars();
	const FString cvarName     = "fontchoice_bigtext_" + langIDString;
	if (FStringCVar *const cv = dynamic_cast<FStringCVar *>(FindCVar(cvarName.GetChars(), nullptr)))
	{
		userFont = V_GetFont(*cv);
	}

	if (!userFont)
	{
		if (lang.CompareNoCase("jp") == 0)
		{
			userFont = V_GetFont(*fontchoice_bigtextJP);
		}
		else if (lang.CompareNoCase("ko") == 0)
		{
			userFont = V_GetFont(*fontchoice_bigtextKO);
		}
		else if (lang.CompareNoCase("ru") == 0 || lang.CompareNoCase("sr") == 0)
		{
			userFont = V_GetFont(*fontchoice_bigtextCYR);
		}
		else if (lang.CompareNoCase("zh-Hans") == 0)
		{
			userFont = V_GetFont(*fontchoice_bigtextSC);
		}
		else if (lang.CompareNoCase("th") == 0)
		{
			userFont = V_GetFont(*fontchoice_bigtextTH);
		}
		else
		{
			userFont = V_GetFont(*fontchoice_bigtext);
		}
	}

	if (userFont)
	{
		return userFont;
	}
	return fallbackIfNoUserChoice;
}

//==========================================================================
//
// RecordTextureColors
//
// Given a 256 entry buffer, sets every entry that corresponds to a color
// used by the texture to 1.
//
//==========================================================================

void RecordTextureColors (FImageSource *pic, uint32_t *usedcolors)
{
	int x;

	auto pixels = pic->GetPalettedPixels(false);
	auto size = pic->GetWidth() * pic->GetHeight();

	for(x = 0;x < size; x++)
	{
		usedcolors[pixels[x]]++;
	}
}

//==========================================================================
//
// RecordLuminosity
//
// Records minimum and maximum luminosity of a texture.
//
//==========================================================================

static void RecordLuminosity(FImageSource* pic, int* minlum, int* maxlum)
{
	auto bitmap = pic->GetCachedBitmap(nullptr, FImageSource::normal);
	auto pixels = bitmap.GetPixels();
	auto size = pic->GetWidth() * pic->GetHeight();

	for (int x = 0; x < size; x++)
	{
		int xx = x * 4;
		if (pixels[xx + 3] > 0)
		{
			int lum = Luminance(pixels[xx + 2], pixels[xx + 1], pixels[xx]);
			if (lum < *minlum) *minlum = lum;
			if (lum > *maxlum) *maxlum = lum;
		}
	}
}

//==========================================================================
//
// RecordAllTextureColors
//
// Given a 256 entry buffer, sets every entry that corresponds to a color
// used by the font.
//
//==========================================================================

void FFont::RecordAllTextureColors(uint32_t *usedcolors)
{
	for (unsigned int i = 0; i < Chars.Size(); i++)
	{
		if (Chars[i].OriginalPic)
		{
			auto pic = Chars[i].OriginalPic->GetTexture()->GetImage();
			if (pic) RecordTextureColors(pic, usedcolors);
		}
	}
}

//==========================================================================
//
// compare
//
// Used for sorting colors by brightness.
//
//==========================================================================

static int compare (const void *arg1, const void *arg2)
{
	if (RPART(GPalette.BaseColors[*((uint8_t *)arg1)]) * 299 +
		GPART(GPalette.BaseColors[*((uint8_t *)arg1)]) * 587 +
		BPART(GPalette.BaseColors[*((uint8_t *)arg1)]) * 114  <
		RPART(GPalette.BaseColors[*((uint8_t *)arg2)]) * 299 +
		GPART(GPalette.BaseColors[*((uint8_t *)arg2)]) * 587 +
		BPART(GPalette.BaseColors[*((uint8_t *)arg2)]) * 114)
		return -1;
	else
		return 1;
}

//==========================================================================
//
// FFont :: GetLuminosity
//
//==========================================================================

int FFont::GetLuminosity (uint32_t *colorsused, TArray<double> &Luminosity, int* minlum, int* maxlum)
{
	double min, max, diver;

	Luminosity.Resize(256);
	Luminosity[0] = 0.0; // [BL] Prevent uninitalized memory
	max = 0.0;
	min = 100000000.0;
	for (int i = 1; i < 256; i++)
	{
		if (colorsused[i])
		{
			Luminosity[i] = GPalette.BaseColors[i].r * 0.299 + GPalette.BaseColors[i].g * 0.587 + GPalette.BaseColors[i].b * 0.114;
			if (Luminosity[i] > max) max = Luminosity[i];
			if (Luminosity[i] < min) min = Luminosity[i];
		}
		else Luminosity[i] = -1;	// this color is not of interest.
	}
	diver = 1.0 / (max - min);
	for (int i = 1; i < 256; i++)
	{
		if (colorsused[i])
		{
			Luminosity[i] = (Luminosity[i] - min) * diver;
		}
	}
	if (minlum) *minlum = int(min);
	if (maxlum) *maxlum = int(max);

	return 256;
}

//==========================================================================
//
// FFont :: GetColorTranslation
//
//==========================================================================

FTranslationID FFont::GetColorTranslation (EColorRange range, PalEntry *color) const
{
	// Single pic fonts do not set up their translation table and must always return 0.
	if (Translations.Size() == 0) return NO_TRANSLATION;
	assert(Translations.Size() == (unsigned)NumTextColors);

	if (noTranslate)
	{
		PalEntry retcolor = PalEntry(255, 255, 255, 255);
		if (range >= 0 && range < NumTextColors && range != CR_UNTRANSLATED)
		{
			retcolor = TranslationColors[range];
			retcolor.a = 255;
		}
		if (color != nullptr) *color = retcolor;
	}
	if (range == CR_UNDEFINED)
		return INVALID_TRANSLATION;
	else if (range >= NumTextColors)
		range = CR_UNTRANSLATED;
	return Translations[range];
}

//==========================================================================
//
// FFont :: GetCharCode
//
// If the character code is in the font, returns it. If it is not, but it
// is lowercase and has an uppercase variant present, return that. Otherwise
// return -1.
//
//==========================================================================

int FFont::GetCharCode(int code, bool needpic) const
{
	int newcode;

	if (code < 0 && code >= -128)
	{
		// regular chars turn negative when the 8th bit is set.
		code &= 255;
	}
	if (special_i && needpic)
	{
		// We need one special case for Turkish: If we have a lowercase-only font (like Raven's) and want to print the capital I, it must map to the dotless ı, because its own glyph will be the dotted i.
		// This checks if the font has no small i, but does define the small dotless ı.
		if (!MixedCase && code == 'I' && LastChar >= 0x131 && Chars['i' - FirstChar].OriginalPic == nullptr && Chars[0x131 - FirstChar].OriginalPic != nullptr)
		{
			return 0x131;
		}
		// a similar check is needed for the small i in allcaps fonts. Here we cannot simply remap to an existing character, so the small dotted i must be placed at code point 0080.
		if (code == 'i' && LastChar >= 0x80 && Chars[0x80 - FirstChar].OriginalPic != nullptr)
		{
			return 0x80;
		}
	}


	if (code >= FirstChar && code <= LastChar && Chars[code - FirstChar].OriginalPic != nullptr)
	{
		return code;
	}

	// Use different substitution logic based on the fonts content:
	// In a font which has both upper and lower case, prefer unaccented small characters over capital ones.
	// In a pure upper-case font, do not check for lower case replacements.
	if (!MixedCase || (lowercaselatinonly && code >= 0x380 && code < 0x500))
	{
		// Try converting lowercase characters to uppercase.
		if (myislower(code))
		{
			code = upperforlower[code];
			if (code >= FirstChar && code <= LastChar && Chars[code - FirstChar].OriginalPic != nullptr)
			{
				return code;
			}
		}
		// Try stripping accents from accented characters.
		while ((newcode = stripaccent(code)) != code)
		{
			code = newcode;
			if (code >= FirstChar && code <= LastChar && Chars[code - FirstChar].OriginalPic != nullptr)
			{
				return code;
			}
		}
	}
	else
	{
		int originalcode = code;

		// Try stripping accents from accented characters. This may repeat to allow multi-step fallbacks.
		while ((newcode = stripaccent(code)) != code)
		{
			code = newcode;
			if (code >= FirstChar && code <= LastChar && Chars[code - FirstChar].OriginalPic != nullptr)
			{
				return code;
			}
		}

		code = originalcode;
		if (myislower(code))
		{
			int upper = upperforlower[code];
			// Stripping accents did not help - now try uppercase for lowercase
			if (upper != code) return GetCharCode(upper, true);
		}

		// Same for the uppercase character. Since we restart at the accented version this must go through the entire thing again.
		while ((newcode = stripaccent(code)) != code)
		{
			code = newcode;
			if (code >= FirstChar && code <= LastChar && Chars[code - FirstChar].OriginalPic != nullptr)
			{
				return code;
			}
		}

	}

	return -1;
}

//==========================================================================
//
// FFont :: GetChar
//
//==========================================================================

FGameTexture *FFont::GetChar (int code, int translation, int *const width) const
{
	code = GetCharCode(code, true);
	int xmove = SpaceWidth;

	if (code >= 0)
	{
		code -= FirstChar;
		xmove = Chars[code].XMove;
	}

	if (width != nullptr)
	{
		*width = xmove;
	}
	if (code < 0) return nullptr;


	assert(Chars[code].OriginalPic->GetUseType() == ETextureType::FontChar);
	return Chars[code].OriginalPic;
}

//==========================================================================
//
// FFont :: GetCharWidth
//
//==========================================================================

int FFont::GetCharWidth (int code) const
{
	//dynamic (ttf) fonts in uzdoom do not use the Chars array, instead unicode codepoints are already tracked
	//by some of the dependencies.
	if (IsValidDynamicFont())
	{
		return DynamicFontAtlas->GetGlyphs().GetGlyphByCodepoint(code).width  * InvSupersampleFactor;
	}
	else
	{
		code = GetCharCode(code, true);
		if (code >= 0)
			return Chars[code - FirstChar].XMove;
	}
	return SpaceWidth;
}

//==========================================================================
//
//
//
//==========================================================================

double GetBottomAlignOffset(FFont *font, int c)
{
	int w;
	auto tex_zero = font->GetChar('0', CR_UNDEFINED, &w);
	auto texc = font->GetChar(c, CR_UNDEFINED, &w);
	double offset = 0;
	if (texc) offset += texc->GetDisplayTopOffset();
	if (tex_zero) offset += -tex_zero->GetDisplayTopOffset() + tex_zero->GetDisplayHeight();
	return offset;
}

bool FFont::CanPrint(const char32_t utf32char) const
{
	if (Type == EFontType::Dynamic)
	{
		return DynamicFontAtlas->GetFont()->GetGlyphIndex(utf32char);
	}
	return false;
}

//==========================================================================
//
// Checks if the font contains proper glyphs for all characters in the string
//
//==========================================================================

bool FFont::CanPrint(const uint8_t *string) const
{
	if (!string) return true;

	if (Type == EFontType::Dynamic)
	{
		FString stripped = (const char*)string;
		stripped         = FString::RemoveColorTags(stripped);
		std::u32string utf32String;
		utf32String.resize(
			simdutf::utf32_length_from_utf8(stripped.GetChars(), std::char_traits<char>::length(stripped.GetChars())), '\0');
		simdutf::convert_utf8_to_utf32(stripped.GetChars(), std::char_traits<char>::length(stripped.GetChars()),
		                               utf32String.data());
		for (auto c : utf32String)
		{
			uint32_t codepoint = c;
			if (!DynamicFontAtlas->GetFont()->GetGlyphIndex(codepoint))
			{
				return false;
			}
		}
		
		return true;
	}

	while (*string)
	{
		auto chr = GetCharFromString(string);
		if (!MixedCase) chr = upperforlower[chr];	// For uppercase-only fonts we shouldn't check lowercase characters.
		if (chr == TEXTCOLOR_ESCAPE)
		{
			// We do not need to check for UTF-8 in here.
			if (*string == '[')
			{
				while (*string != '\0' && *string != ']')
				{
					++string;
				}
			}
			if (*string != '\0')
			{
				++string;
			}
			continue;
		}
		else if (chr != '\n')
		{
			int cc = GetCharCode(chr, false);
			if (chr != cc && myiswalpha(chr) && cc != getAlternative(chr))
			{
				return false;
			}
		}
	}

	return true;
}

//==========================================================================
//
// Find string width using this font
//
//==========================================================================

int FFont::StringWidth(const uint8_t *string, int spacing) const
{
	if (IsValidDynamicFont())
	{
		std::vector<IntermediateDrawString> drawStrings;
		drawStrings.reserve(4);
		std::u32string utf32String;
		utf32String.resize(
			simdutf::utf32_length_from_utf8((const char *)string, std::char_traits<uint8_t>::length(string)), '\0');
		simdutf::convert_utf8_to_utf32((const char *)string, std::char_traits<uint8_t>::length(string),
		                               utf32String.data());
		ParseIntoIntermediateDrawStrings(utf32String, this, 0, drawStrings);

		float totalWidth = 0.0f;
		for (auto &s : drawStrings)
		{
			totalWidth += Trex::TextShaper::Measure(s.TrexGlyphs).width * (float)InvSupersampleFactor;
		}
		return std::ceil(totalWidth);
	}

	int w = 0;
	int maxw = 0;

	while (*string)
	{
		auto chr = GetCharFromString(string);
		if (chr == TEXTCOLOR_ESCAPE)
		{
			// We do not need to check for UTF-8 in here.
			if (*string == '[')
			{
				while (*string != '\0' && *string != ']')
				{
					++string;
				}
			}
			if (*string != '\0')
			{
				++string;
			}
			continue;
		}
		else if (chr == '\n')
		{
			if (w > maxw)
				maxw = w;
			w = 0;
		}
		else if (spacing >= 0)
		{
			w += GetCharWidth(chr) + GlobalKerning + spacing;
		}
		else
		{
			w -= spacing;
		}
	}

	return max(maxw, w);
}

//only supports dynamic fonts.
int FFont::StringWidthUTF32(const std::u32string_view string) const
{
	if (IsValidDynamicFont())
	{
		std::vector<IntermediateDrawString> drawStrings;
		drawStrings.reserve(4);
		ParseIntoIntermediateDrawStrings(string, this, 0, drawStrings);

		float totalWidth = 0.0f;
		for (auto &s : drawStrings)
		{
			totalWidth += Trex::TextShaper::Measure(s.TrexGlyphs).xAdvance * (float)InvSupersampleFactor;
		}
		return std::ceil(totalWidth);
	}
	return 0;
}

//==========================================================================
//
// Get the largest ascender in the first line of this text.
//
//==========================================================================

int FFont::GetMaxAscender(const uint8_t* string) const
{
	int retval = 0;

	if (IsValidDynamicFont())
	{
		return GetDynamicFontAtlas()->GetFont()->GetMetrics().ascender;
	}

	while (*string)
	{
		auto chr = GetCharFromString(string);
		if (chr == TEXTCOLOR_ESCAPE)
		{
			// We do not need to check for UTF-8 in here.
			if (*string == '[')
			{
				while (*string != '\0' && *string != ']')
				{
					++string;
				}
			}
			if (*string != '\0')
			{
				++string;
			}
			continue;
		}
		else if (chr == '\n')
		{
			break;
		}
		else
		{
			auto ctex = GetChar(chr, CR_UNTRANSLATED, nullptr);
			if (ctex)
			{
				auto offs = int(ctex->GetDisplayTopOffset());
				if (offs > retval) retval = offs;
			}
		}
	}

	return retval;
}

//==========================================================================
//
// FFont :: LoadTranslations
//
//==========================================================================

void FFont::LoadTranslations()
{
	unsigned int count = min<unsigned>(Chars.Size(), LastChar - FirstChar + 1);

	if (count == 0) return;
	int minlum = 255, maxlum = 0;
	for (unsigned int i = 0; i < count; i++)
	{
		if (Chars[i].OriginalPic)
		{
			auto pic = Chars[i].OriginalPic->GetTexture()->GetImage();
			RecordLuminosity(pic, &minlum, &maxlum);
		}
	}

	if (MinLum >= 0 && MinLum < minlum) minlum = MinLum;
	if (MaxLum > maxlum) maxlum = MaxLum;

	// Here we can set everything to a luminosity translation.
	Translations.Resize(NumTextColors);
	for (int i = 0; i < NumTextColors; i++)
	{
		if (i == CR_UNTRANSLATED) Translations[i] = NO_TRANSLATION;
 		else Translations[i] = MakeLuminosityTranslation(i*2 + TranslationType, minlum, maxlum);
	}
}

bool FFont::CanBeSubstitutedWithDynamic() const noexcept
{
	return !IsValidDynamicFont() &&
		(Chars.size() > 1 || LastChar - FirstChar > 1) &&
		(LastChar - FirstChar > 0) &&
		this != SymbolsFont;
}

//==========================================================================
//
// FFont :: FFont - default constructor
//
//==========================================================================

FFont::FFont (int lump, FName nm)
{
	FirstChar = LastChar = 0;
	Next = FirstFont;
	FirstFont = this;
	Lump = lump;
	FontName = nm;
	Cursor = '_';
	noTranslate = false;
}

class FTrexAtlasImageSource : public FImageSource
{
	const Trex::Atlas::Bitmap *TrexBitmap = nullptr;
  public:
	FTrexAtlasImageSource(const Trex::Atlas::Bitmap * tBmp, int width, int height)
	{
		TrexBitmap = tBmp;
		Width     = width;
		Height    = height;
	}

	virtual int CopyPixels(FBitmap *dest, int conversion, int frame = 0) override
	{
		if (!TrexBitmap)
		{
			return 0;
		}

		auto& bmp = *TrexBitmap;
		auto channels = bmp.Channels();
		auto  format   = CF_RGBA;
		if (channels == 1)
		{
			format = CF_IA;
		}
		dest->CopyPixelDataRGB(0, 0, bmp.Data().data(), bmp.Width(), bmp.Height(),
		                       channels, bmp.Width() * channels, 0, format);
		return 0;
	}
};

FFont::FFont(const char *fontname, Trex::Atlas* fontAtlas, const int superSampleScale)
{
	noTranslate    = true;
	GlobalKerning  = false;
	SpaceWidth     = 0;

	Type = EFontType::Dynamic;
	DynamicFontAtlas = fontAtlas;
	DynamicTextShaper = new Trex::TextShaper(*fontAtlas);

	FirstChar = LastChar = 0;
	Next                 = FirstFont;
	FirstFont            = this;
	Lump                 = -1;
	FontName             = FName(fontname);
	Cursor               = '_';
	noTranslate          = false;

	const Trex::Atlas::Bitmap& trexBitmap = fontAtlas->GetBitmap();
	FTrexAtlasImageSource* src = new FTrexAtlasImageSource(&trexBitmap, trexBitmap.Width(), trexBitmap.Height());
	FImageTexture *img = new FImageTexture(src);
	img->SetSize(trexBitmap.Width(), trexBitmap.Height());
	FGameTexture  *tex   = MakeGameTexture(img, nullptr, ETextureType::FontChar);
	tex->SetSize(trexBitmap.Width(), trexBitmap.Height());
	TexMan.AddGameTexture(tex);
	DynamicFontAtlasTexture = tex;
	assert(superSampleScale > 0.0);
	InvSupersampleFactor          = 1.0 / (superSampleScale);

	FontHeight = fontAtlas->GetFont()->GetMetrics().height * InvSupersampleFactor;
}

FFont *FFont::GetDynamicFontFallbackForChar32(char32_t srcChar) const
{
	//try special writing system fonts and see if they can handle it
	auto JPNFont = V_GetFont(*fontoverride_FallbackJP);
	auto KRFont  = V_GetFont(*fontoverride_FallbackKR);
	if (JPNFont->CanPrint(srcChar))
	{
		return JPNFont;
	}
	else if (KRFont->CanPrint(srcChar)) 
	{
		return KRFont;
	}
	return V_GetFont(*fontoverride_Fallback);
}

//==========================================================================
//
// FFont :: FixXMoves
//
// If a font has gaps in its characters, set the missing characters'
// XMoves to either SpaceWidth or the unaccented or uppercase variant's
// XMove. Missing XMoves must be initialized with INT_MIN beforehand.
//
//==========================================================================

void FFont::FixXMoves()
{
	if (FirstChar < 'a' && LastChar >= 'z')
	{
		MixedCase = true;
		// First check if this is a mixed case font.
		// For this the basic Latin small characters all need to be present.
		for (int i = 'a'; i <= 'z'; i++)
			if (Chars[i - FirstChar].OriginalPic == nullptr)
			{
				MixedCase = false;
				break;
			}
	}

	for (int i = 0; i <= LastChar - FirstChar; ++i)
	{
		if (Chars[i].XMove == INT_MIN)
		{
			// Try an uppercase character.
			if (myislower(i + FirstChar))
			{
				int upper = upperforlower[FirstChar + i];
				if (upper >= FirstChar && upper <= LastChar )
				{
					Chars[i].XMove = Chars[upper - FirstChar].XMove;
					continue;
				}
			}
			// Try an unnaccented character.
			int noaccent = stripaccent(i + FirstChar);
			if (noaccent != i + FirstChar)
			{
				noaccent -= FirstChar;
				if (noaccent >= 0)
				{
					Chars[i].XMove = Chars[noaccent].XMove;
					continue;
				}
			}
			Chars[i].XMove = SpaceWidth;
		}
		if (Chars[i].OriginalPic)
		{
			int ofs = (int)Chars[i].OriginalPic->GetDisplayTopOffset();
			if (ofs > Displacement) Displacement = ofs;
		}
	}
}


void FFont::ClearOffsets()
{
	for (auto& c : Chars) if (c.OriginalPic) c.OriginalPic->SetOffsets(0, 0);
}
