/*
** v_font.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#pragma once

#include "filesystem.h"
#include "vectors.h"
#include "palentry.h"
#include "name.h"
#include "palettecontainer.h"
#include "Trex/Atlas.hpp"
#include "Trex/TextShaper.hpp"

class FGameTexture;
struct FRemapTable;
class FFont;

FFont* V_GetFont(const char* fontname, const char* fontlumpname = nullptr);

enum EColorRange : int
{
	CR_UNDEFINED = -1,
	CR_NATIVEPAL = -1,
	CR_BRICK,
	CR_TAN,
	CR_GRAY,
	CR_GREY = CR_GRAY,
	CR_GREEN,
	CR_BROWN,
	CR_GOLD,
	CR_RED,
	CR_BLUE,
	CR_ORANGE,
	CR_WHITE,
	CR_YELLOW,
	CR_UNTRANSLATED,
	CR_BLACK,
	CR_LIGHTBLUE,
	CR_CREAM,
	CR_OLIVE,
	CR_DARKGREEN,
	CR_DARKRED,
	CR_DARKBROWN,
	CR_PURPLE,
	CR_DARKGRAY,
	CR_CYAN,
	CR_ICE,
	CR_FIRE,
	CR_SAPPHIRE,
	CR_TEAL,
	NUM_TEXT_COLORS,
};

extern int NumTextColors;

struct IntermediateDrawString
{
	std::basic_string<char32_t> StringUTF32;
	Trex::ShapedGlyphs          TrexGlyphs;
	std::vector<int>            Colors;
	std::vector<uint32_t>            Codepoints;
	const FFont                      *Font;
};

using GlyphSet = TMap<int, FGameTexture*>;

class FFont
{
	friend void V_LoadTranslations();
public:

	enum EFontType
	{
		Unknown,
		Folder,
		Multilump,
		Fon1,
		Fon2,
		BMF,
		Custom,
		Dynamic
	};

	FFont(const char *fontname, const char *nametemplate, const char *filetemplate, int first, int count, int base,
	      int fdlump, int spacewidth = -1, bool notranslate = false, bool iwadonly = false, bool doomtemplate = false,
	      GlyphSet *baseGlpyphs = nullptr);
	FFont(int lump, FName nm = NAME_None);
	explicit FFont(const char *fontname, Trex::Atlas* fontAtlas, const int supersampleScale);
	virtual ~FFont ();

	virtual FGameTexture *GetChar (int code, int translation, int *const width) const;
	virtual int GetCharWidth (int code) const;
	FTranslationID GetColorTranslation (EColorRange range, PalEntry *color = nullptr) const;
	int GetLump() const { return Lump; }
	int GetSpaceWidth () const { return SpaceWidth; }
	int GetHeight () const { return FontHeight; }
	int GetDefaultKerning () const { return GlobalKerning; }
	int GetMaxAscender(const uint8_t* text) const;
	int GetMaxAscender(const char* text) const { return GetMaxAscender((uint8_t*)text); }
	int GetMaxAscender(const FString &text) const { return GetMaxAscender((uint8_t*)text.GetChars()); }
	virtual void LoadTranslations();
	FName GetName() const { return FontName; }
	[[nodiscard]] bool CanBeSubstitutedWithDynamic() const noexcept;

	static FFont *FindFont(FName fontname);

	static void UpdateAdvFontMappingTables();
	static FFont *GetDynamicSubstitutionForStaticFont(FFont *const fontToSub);
	static void   MakeFontChoiceCVARs();

	// standard font choices
	//TODO: this seems not super extensible - what if mods want to add semantic font choices?
	static FFont *GetSmallTextFont(FFont* fallbackIfNoUserChoice);
	static FFont *GetTitleFont(FFont* fallbackIfNoUserChoice);
	static FFont *GetDescriptionFont(FFont* fallbackIfNoUserChoice);
	static FFont *GetConsoleFont(FFont* fallbackIfNoUserChoice);
	static FFont *GetBigTextFont(FFont* fallbackIfNoUserChoice);

	static const FFont* GetFontListHead()
	{
		return FirstFont;
	}

	const FFont* GetNextFont() const
	{
		return Next;
	}

	// Return width of string in pixels (unscaled)
	int StringWidth (const uint8_t *str, int spacing = 0) const;
	inline int StringWidth (const char *str, int spacing = 0) const { return StringWidth ((const uint8_t *)str, spacing); }
	inline int StringWidth (const FString &str, int spacing = 0) const { return StringWidth ((const uint8_t *)str.GetChars(), spacing); }

	[[nodiscard]] int StringWidthUTF32(const std::u32string_view str) const;

	bool CanPrint(const char32_t utf32char) const;

	// Checks if the font contains all characters to print this text.
	bool CanPrint(const uint8_t *str) const;
	inline bool CanPrint(const char *str) const { return CanPrint((const uint8_t *)str); }
	inline bool CanPrint(const FString &str) const { return CanPrint((const uint8_t *)str.GetChars()); }

	inline FFont* AltFont()
	{
		if (AltFontName != NAME_None) return V_GetFont(AltFontName.GetChars());
		return nullptr;
	}

	int GetCharCode(int code, bool needpic) const;
	char GetCursor() const { return Cursor; }
	void SetCursor(char c) { Cursor = c; }
	void SetKerning(int c) { GlobalKerning = c; }
	void SetHeight(int c) { FontHeight = c; }
	void ClearOffsets();
	bool NoTranslate() const { return noTranslate; }
	virtual void RecordAllTextureColors(uint32_t *usedcolors);
	void CheckCase();
	void SetName(FName nm) { FontName = nm; }

	int GetDisplacement() const { return Displacement; }

	static int GetLuminosity(uint32_t* colorsused, TArray<double>& Luminosity, int* minlum = nullptr, int* maxlum = nullptr);
	EFontType GetType() const { return Type; }

	inline bool IsValidDynamicFont() const
	{
		return Type == EFontType::Dynamic && DynamicFontAtlas && DynamicFontAtlasTexture;
	}

	inline Trex::Atlas *GetDynamicFontAtlas() const
	{
		return DynamicFontAtlas;
	}

	inline FGameTexture* GetDynamicFontAtlasTexture() const
	{
		return DynamicFontAtlasTexture;
	}

	inline double GetInvSupersampleScale() const
	{
		return InvSupersampleFactor;
	}

	inline Trex::TextShaper* GetDynamicTextShaper() const
	{
		return DynamicTextShaper;
	}

	FFont *GetDynamicFontFallbackForChar32(char32_t srcChar) const;

	friend void V_InitCustomFonts();

	void CopyFrom(const FFont& other)
	{
		Type = other.Type;
		FirstChar = other.FirstChar;
		LastChar = other.LastChar;
		SpaceWidth = other.SpaceWidth;
		FontHeight = other.FontHeight;
		GlobalKerning = other.GlobalKerning;
		TranslationType = other.TranslationType;
		Displacement = other.Displacement;
		Cursor = other.Cursor;
		noTranslate = other.noTranslate;
		MixedCase = other.MixedCase;
		forceremap = other.forceremap;
		Chars = other.Chars;
		Translations = other.Translations;
		lowercaselatinonly = other.lowercaselatinonly;
		Lump = other.Lump;
	}

	static inline std::vector<const FFont *> &GetRemappableFonts()
	{
		return RemappableFonts;
	}

protected:

	void FixXMoves();

	void ReadSheetFont(std::vector<FileSys::FolderEntry> &folderdata, int width, int height, const DVector2 &Scale);

	EFontType Type = EFontType::Unknown;
	FName AltFontName = NAME_None;
	int FirstChar, LastChar;
	int SpaceWidth;
	int FontHeight;
	int GlobalKerning;
	int TranslationType = 0;
	int Displacement = 0;
	int16_t MinLum = -1, MaxLum = -1;
	char Cursor;
	bool noTranslate = false;
	bool MixedCase = false;
	bool forceremap = false;
	bool lowercaselatinonly = false;
	struct CharData
	{
		FGameTexture *OriginalPic = nullptr;
		int XMove = INT_MIN;
	};
	TArray<CharData> Chars;
	TArray<FTranslationID> Translations;

	int Lump;
	FName FontName = NAME_None;
	FFont *Next;

	//DYNAMIC FONTS
	Trex::Atlas *DynamicFontAtlas = nullptr;
	Trex::TextShaper *DynamicTextShaper             = nullptr;
	FGameTexture *DynamicFontAtlasTexture = nullptr;
	double        InvSupersampleFactor          = 1.0 / 3.0;
	//DYNAMIC FONTS

	static FFont *FirstFont;
	friend struct FontsDeleter;

	friend void V_ClearFonts();
	friend void V_InitFonts();

	static inline std::vector<const FFont *>     RemappableFonts;
};

void ParseIntoIntermediateDrawStrings(const std::u32string_view utf32SrcString, const FFont *font, int normalcolor,
                                      std::vector<IntermediateDrawString> &outStrings);

extern FFont *SmallFont, *SmallFont2, *BigFont, *BigUpper, *ConFont, *SymbolsFont, *IntermissionFont, *NewConsoleFont, *NewSmallFont, *CurrentConsoleFont, *OriginalSmallFont, *AlternativeSmallFont, *OriginalBigFont, *AlternativeBigFont;

void V_InitFonts();
void V_ClearFonts();
EColorRange V_FindFontColor (FName name);
PalEntry V_LogColorFromColorRange (EColorRange range);
EColorRange V_ParseFontColor (const uint8_t *&color_value, int normalcolor, int boldcolor);
void V_InitFontColors();
char* CleanseString(char* str);
void V_ApplyLuminosityTranslation(const LuminosityTranslationDesc& lum, uint8_t* pixel, int size);
void V_LoadTranslations();
class FBitmap;
