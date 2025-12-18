/*
** v_drawtext.cpp
**
** Draws text to a canvas. Also has a text line-breaker thingy.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2016 Christoph Oelckers
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

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <wctype.h>

#include "v_text.h"
#include "utf8.h"
#include "v_draw.h"
#include "gstrings.h"
#include "vm.h"
#include "printf.h"

#include "Trex/TextShaper.hpp"
#include "Trex/BitmapHelpers.hpp"
#include "Trex/TextShaper.hpp"
#include <type_traits>
#include <span>
#include <string>
#include "Trex/Atlas.hpp"
#include <vector>
#include <string_view>
#include "simdutf.h"

int ListGetInt(VMVa_List &tags);


//==========================================================================
//
// Create a texture from a text in a given font.
//
//==========================================================================
#if 0
FGameTexture * BuildTextTexture(FFont *font, const char *string, int textcolor)
{
	int 		w;
	const uint8_t *ch;
	int 		cx;
	int 		cy;
	int			trans = -1;
	int			kerning;
	FGameTexture *pic;

	kerning = font->GetDefaultKerning();

	ch = (const uint8_t *)string;
	cx = 0;
	cy = 0;


	IntRect box;

	while (auto c = GetCharFromString(ch))
	{
		if (c == TEXTCOLOR_ESCAPE)
		{
			// Here we only want to measure the texture so just parse over the color.
			V_ParseFontColor(ch, 0, 0);
			continue;
		}

		if (c == '\n')
		{
			cx = 0;
			cy += font->GetHeight();
			continue;
		}

		if (nullptr != (pic = font->GetChar(c, CR_UNTRANSLATED, &w, nullptr)))
		{
			auto img = pic->GetImage();
			auto offsets = img->GetOffsets();
			int x = cx - offsets.first;
			int y = cy - offsets.second;
			int ww = img->GetWidth();
			int h = img->GetHeight();

			box.AddToRect(x, y);
			box.AddToRect(x + ww, y + h);
		}
		cx += (w + kerning);
	}

	cx = -box.left;
	cy = -box.top;

	TArray<TexPart> part(strlen(string));

	while (auto c = GetCharFromString(ch))
	{
		if (c == TEXTCOLOR_ESCAPE)
		{
			EColorRange newcolor = V_ParseFontColor(ch, textcolor, textcolor);
			if (newcolor != CR_UNDEFINED)
			{
				trans = font->GetColorTranslation(newcolor);
				textcolor = newcolor;
			}
			continue;
		}

		if (c == '\n')
		{
			cx = 0;
			cy += font->GetHeight();
			continue;
		}

		if (nullptr != (pic = font->GetChar(c, textcolor, &w, nullptr)))
		{
			auto img = pic->GetImage();
			auto offsets = img->GetOffsets();
			int x = cx - offsets.first;
			int y = cy - offsets.second;

			auto &tp = part[part.Reserve(1)];

			tp.OriginX = x;
			tp.OriginY = y;
			tp.Image = img;
			tp.Translation = range;
		}
		cx += (w + kerning);
	}
	FMultiPatchTexture *image = new FMultiPatchTexture(box.width, box.height, part, false, false);
	image->SetOffsets(-box.left, -box.top);
	FImageTexture *tex = new FImageTexture(image, "");
	tex->SetUseType(ETextureType::MiscPatch);
	TexMan.AddTexture(tex);
	return tex;
}
#endif


//==========================================================================
//
// DrawChar
//
// Write a single character using the given font
//
//==========================================================================

void DrawChar(F2DDrawer *drawer, FFont* font, int normalcolor, double x, double y, int character, int tag_first, ...)
{
	if (font == NULL)
		return;

	if (normalcolor >= NumTextColors)
		normalcolor = CR_UNTRANSLATED;

	FGameTexture* pic;
	int dummy;

	if (NULL != (pic = font->GetChar(character, normalcolor, &dummy)))
	{
		DrawParms parms;
		Va_List tags;
		va_start(tags.list, tag_first);
		bool res = ParseDrawTextureTags(drawer, pic, x, y, tag_first, tags, &parms, DrawTexture_Normal);
		va_end(tags.list);
		if (!res)
		{
			return;
		}
		bool palettetrans = (normalcolor == CR_NATIVEPAL && parms.TranslationId != NO_TRANSLATION);
		PalEntry color = 0xffffffff;
		if (!palettetrans) parms.TranslationId = font->GetColorTranslation((EColorRange)normalcolor, &color);
		parms.color = PalEntry((color.a * parms.color.a) / 255, (color.r * parms.color.r) / 255, (color.g * parms.color.g) / 255, (color.b * parms.color.b) / 255);
		drawer->AddTexture(pic, parms);
	}
}

void DrawChar(F2DDrawer *drawer,  FFont *font, int normalcolor, double x, double y, int character, VMVa_List &args)
{
	if (font == NULL)
		return;

	if (normalcolor >= NumTextColors)
		normalcolor = CR_UNTRANSLATED;

	FGameTexture *pic;
	int dummy;

	if (NULL != (pic = font->GetChar(character, normalcolor, &dummy)))
	{
		DrawParms parms;
		uint32_t tag = ListGetInt(args);
		bool res = ParseDrawTextureTags(drawer, pic, x, y, tag, args, &parms, DrawTexture_Normal);
		if (!res) return;
		bool palettetrans = (normalcolor == CR_NATIVEPAL && parms.TranslationId != NO_TRANSLATION);
		PalEntry color = 0xffffffff;
		if (!palettetrans) parms.TranslationId = font->GetColorTranslation((EColorRange)normalcolor, &color);
		parms.color = PalEntry((color.a * parms.color.a) / 255, (color.r * parms.color.r) / 255, (color.g * parms.color.g) / 255, (color.b * parms.color.b) / 255);
		drawer->AddTexture(pic, parms);
	}
}

DEFINE_ACTION_FUNCTION(_Screen, DrawChar)
{
	PARAM_PROLOGUE;
	PARAM_POINTER(font, FFont);
	PARAM_INT(cr);
	PARAM_FLOAT(x);
	PARAM_FLOAT(y);
	PARAM_INT(chr);

	PARAM_VA_POINTER(va_reginfo)	// Get the hidden type information array

	if (!twod->HasBegun2D()) ThrowAbortException(X_OTHER, "Attempt to draw to screen outside a draw function");
	VMVa_List args = { param + 5, 0, numparam - 6, va_reginfo + 5 };
	DrawChar(twod, font, cr, x, y, chr, args);
	return 0;
}

DEFINE_ACTION_FUNCTION(FCanvas, DrawChar)
{
	PARAM_SELF_PROLOGUE(FCanvas);
	PARAM_POINTER(font, FFont);
	PARAM_INT(cr);
	PARAM_FLOAT(x);
	PARAM_FLOAT(y);
	PARAM_INT(chr);

	PARAM_VA_POINTER(va_reginfo)	// Get the hidden type information array

	VMVa_List args = { param + 6, 0, numparam - 7, va_reginfo + 6 };
	DrawChar(&self->Drawer, font, cr, x, y, chr, args);
	self->Tex->NeedUpdate();
	return 0;
}

//==========================================================================
//
// DrawText
//
// Write a string using the given font
//
//==========================================================================

// This is only needed as a dummy. The code using wide strings does not need color control.
EColorRange V_ParseFontColor(const char32_t *&color_value, int normalcolor, int boldcolor) { return CR_UNTRANSLATED; }

struct IntermediateDrawString
{
	std::basic_string<char32_t> StringUTF32;
	Trex::ShapedGlyphs TrexGlyphs;
	std::vector<int>  Colors;
	std::vector<int>  Codepoints;
	FFont*                   Font;
};

//split the strings into substrings based on what glyphs are supported by the target fonts.
//shape each substring separately.
void ParseIntoIntermediateDrawStrings(const std::u32string_view utf32SrcString, FFont* font, int normalcolor, std::vector<IntermediateDrawString> &outStrings)
{
	outStrings.clear();
	
	auto* currentDrawString = &outStrings.emplace_back(IntermediateDrawString());
	currentDrawString->Font = font;
	bool insideEscapeSequence = false;
	bool insideNamedColorTagSequence = false;
	int currentcolor = 0;
	int boldcolor = normalcolor ? normalcolor - 1 : NumTextColors - 1;
	bool isInFallback                = false;
	std::u32string colorSubStr;
	for (int i =0; i < utf32SrcString.size(); ++i)
	{
		const char32_t& srcChar = utf32SrcString[i];
		if (srcChar == TEXTCOLOR_ESCAPE)
		{
			insideEscapeSequence = true;
			continue;
		}

		if (insideEscapeSequence && !insideNamedColorTagSequence && srcChar != '[')
		{
			if (srcChar == '+')
			{
				int intVal           = FindCVar("msgmidcolor", nullptr)->ToInt();
				currentcolor         = V_LogColorFromColorRange((EColorRange)boldcolor);
				insideEscapeSequence = false;
				continue;
			}
			else if (srcChar == '-')
			{
				currentcolor = V_LogColorFromColorRange((EColorRange)normalcolor);
				insideEscapeSequence = false;
				continue;
			}
			else if (srcChar == '*')
			{
				// use chat color
				int intVal   = FindCVar("msg3color", nullptr)->ToInt();
				currentcolor         = V_LogColorFromColorRange((EColorRange)intVal);
				insideEscapeSequence = false;
				continue;
			}
			else if (srcChar == '!')
			{
				//use team chat color
				int intVal           = FindCVar("msg4color", nullptr)->ToInt();
				currentcolor         = V_LogColorFromColorRange((EColorRange)intVal);
				insideEscapeSequence   = false;
				continue;
			}
			else if (srcChar != '[')
			{
				// each letter a,b,c etc maps to 1,2,3 .. and so on in EColorRange
				EColorRange colorRange = (EColorRange)((int)srcChar - (int)'a');
				currentcolor           = V_LogColorFromColorRange(colorRange);
				insideEscapeSequence   = false;
				continue;
			}
		}
		else if (insideEscapeSequence && srcChar == '[')
		{
			insideNamedColorTagSequence = true;
			continue;
		}
		else if (insideNamedColorTagSequence && srcChar != ']')
		{
			colorSubStr += srcChar;
			continue;
		}
		else if (insideNamedColorTagSequence && srcChar == ']')
		{
			insideEscapeSequence        = false;
			insideNamedColorTagSequence = false;
			std::string utf8ColorSubStr;
			utf8ColorSubStr.resize(
				simdutf::utf8_length_from_utf32(std::span<const char32_t>(colorSubStr.cbegin(), colorSubStr.cend())));
			simdutf::convert_utf32_to_utf8(colorSubStr.data(), colorSubStr.size(), utf8ColorSubStr.data());
			EColorRange newcolor        = V_FindFontColor(FName(utf8ColorSubStr.data()));
			colorSubStr.clear();
			if (newcolor != CR_UNDEFINED)
			{
				currentcolor = V_LogColorFromColorRange(newcolor);
				continue;
			}
			
			continue;
		}
		else if (insideEscapeSequence && srcChar != TEXTCOLOR_ESCAPE && srcChar != '[')
		{
			insideEscapeSequence        = false;
			insideNamedColorTagSequence = false;
		}
		else if (insideEscapeSequence)
		{
			continue;
		}
		//is this codepoint supported by the target font? If no, split
		//TODO: if we were writing to fallback and we encounter chars we can render with the desired font again,
		//go back to writing with the desired font
		if (!font->CanPrint(srcChar))
		{
			currentDrawString = &outStrings.emplace_back(IntermediateDrawString());
			currentDrawString->Font = font->GetDynamicFontFallback();
			isInFallback            = true;
		}
		else if (isInFallback && font->CanPrint(srcChar))
		{
			currentDrawString       = &outStrings.emplace_back(IntermediateDrawString());
			currentDrawString->Font = font;
			isInFallback            = false;
		}
		currentDrawString->StringUTF32 += srcChar;
		currentDrawString->Codepoints.push_back(srcChar);
		currentDrawString->Colors.push_back(currentcolor);
	}

	for (auto &s : outStrings)
	{
		auto res = s.Font->GetDynamicTextShaper()->ShapeUtf32(
			std::span<const char32_t>(s.StringUTF32.begin(), s.StringUTF32.end()));
		s.TrexGlyphs = res;
	}
}

template<class chartype>
void DrawTextCommon(F2DDrawer *drawer, FFont *font, int normalcolor, double x, double y, const chartype *string, DrawParms &parms)
{
	int 		w;
	const chartype *ch;
	int 		c;
	double 		cx;
	double 		cy;
	int			boldcolor;
	FTranslationID			trans = INVALID_TRANSLATION;
	int			kerning;
	FGameTexture *pic;

	double scalex = parms.scalex * parms.patchscalex;
	double scaley = parms.scaley * parms.patchscaley;

	if (font && font->IsValidDynamicFont())
	{
		//convert string to utf32 for straightforward and debuggable string parsing.
		std::vector<IntermediateDrawString> DrawStrings;
		std::u32string utf32String;
		
		if constexpr (std::is_same_v<chartype, uint8_t> || std::is_same_v<chartype, char> ||
		              std::is_same_v<chartype, char8_t>)
		{
			utf32String.resize(
				simdutf::utf32_length_from_utf8((const char *)string, std::char_traits<chartype>::length(string)),
				'\0');
			simdutf::convert_utf8_to_utf32((const char*)string, std::char_traits<chartype>::length(string), utf32String.data());
		}
		else if constexpr (std::is_same_v<chartype, char16_t>)
		{
			utf32String.resize(
				simdutf::utf32_length_from_utf16((const char16_t*)string, std::char_traits<chartype>::length(string)),
				'\0');
			simdutf::convert_utf16_to_utf32(string, std::char_traits<chartype>::length(string), utf32String);
		}
		else if constexpr (std::is_same_v<chartype, char32_t>)
		{
			utf32String = string;
		}
		assert(simdutf::validate_utf32(utf32String.c_str(), utf32String.length()));
		ParseIntoIntermediateDrawStrings(utf32String, font, normalcolor, DrawStrings);
		
		double cursorx = x;
		double cursory = y;
		for (auto &s : DrawStrings)
		{
			DrawParms           atlasFragmentDrawParms      = parms;
			const double        shrinkScale                 = s.Font->GetInvSupersampleScale();
			const double        baseFontHeight              = s.Font->GetHeight();
			FGameTexture *const atlasTexture                = s.Font->GetDynamicFontAtlasTexture();
			const Trex::Atlas  &atlas                       = *s.Font->GetDynamicFontAtlas();
			Trex::TextShaper   &shaper                      = *s.Font->GetDynamicTextShaper();
			scalex = atlasFragmentDrawParms.scalex * atlasFragmentDrawParms.patchscalex;
			scaley = atlasFragmentDrawParms.scaley * atlasFragmentDrawParms.patchscaley;
			
			for (int i = 0; i < s.TrexGlyphs.size(); ++i)
			{
				const Trex::ShapedGlyph &g = s.TrexGlyphs[i];
				const double cx           = cursorx + (shrinkScale * scalex) * (g.xOffset + g.info.bearingX);
				const double heightAdjust = 1.0 / s.Font->GetInvSupersampleScale();
				const double cy =
					cursory + (scaley * shrinkScale) * (baseFontHeight * (heightAdjust) + g.yOffset - g.info.bearingY);
				const double srcx = (double)g.info.x / (double)atlasTexture->GetDisplayWidth();
				const double srcy = (double)g.info.y / (double)atlasTexture->GetDisplayHeight();
				const double srcw = (double)g.info.width / (double)atlasTexture->GetDisplayWidth();
				const double srch = (double)g.info.height / (double)atlasTexture->GetDisplayHeight();
				SetTextureParmsSubrect(drawer, &atlasFragmentDrawParms, atlasTexture, cx, cy, srcx, srcy, srcw, srch);
				atlasFragmentDrawParms.masked  = true;
				atlasFragmentDrawParms.fortext = true;
				atlasFragmentDrawParms.bilinear   = 1;
				atlasFragmentDrawParms.destwidth *= (shrinkScale);
				atlasFragmentDrawParms.destheight *= (shrinkScale);
				if (s.Colors[i] != 0)
				{
					atlasFragmentDrawParms.color = s.Colors[i];
					atlasFragmentDrawParms.color.a = 255;
				}

				// TODO: cvar
				const bool drawDropShadow = true;
				if (drawDropShadow)
				{
					auto shadowAtlasFragmentDrawParms = atlasFragmentDrawParms;
					double textScale                    = (double)atlasFragmentDrawParms.destheight / baseFontHeight;
					shadowAtlasFragmentDrawParms.x += (1.33*scalex*textScale);
					shadowAtlasFragmentDrawParms.y += (1.33*scaley*textScale);
					shadowAtlasFragmentDrawParms.color = MAKEARGB(255, 33, 33, 33);
					drawer->AddTexture(atlasTexture, shadowAtlasFragmentDrawParms);
				}

				drawer->AddTexture(atlasTexture, atlasFragmentDrawParms);
				cursorx += (g.xAdvance) * scalex * shrinkScale;
				cursory += (g.yAdvance) * scaley * shrinkScale;
			}
		}
		return;
	}

	if (parms.celly == 0) parms.celly = font->GetHeight() + 1;
	parms.celly = int (parms.celly * scaley);

	bool palettetrans = (normalcolor == CR_NATIVEPAL && parms.TranslationId != NO_TRANSLATION);

	if (normalcolor >= NumTextColors)
		normalcolor = CR_UNTRANSLATED;
	boldcolor = normalcolor ? normalcolor - 1 : NumTextColors - 1;

	PalEntry colorparm = parms.color;
	PalEntry color = 0xffffffff;
	trans = palettetrans? INVALID_TRANSLATION : font->GetColorTranslation((EColorRange)normalcolor, &color);
	parms.color = PalEntry(colorparm.a, (color.r * colorparm.r) / 255, (color.g * colorparm.g) / 255, (color.b * colorparm.b) / 255);

	kerning = font->GetDefaultKerning();

	ch = string;
	cx = x;
	cy = y;

	if (parms.monospace == EMonospacing::CellCenter)
		cx += parms.spacing / 2;
	else if (parms.monospace == EMonospacing::CellRight)
		cx += parms.spacing;


	auto currentcolor = normalcolor;
	while (ch - string < parms.maxstrlen)
	{
		c = GetCharFromString(ch);
		if (!c)
			break;

		if (c == TEXTCOLOR_ESCAPE)
		{
			EColorRange newcolor = V_ParseFontColor(ch, normalcolor, boldcolor);
			if (newcolor != CR_UNDEFINED)
			{
				trans = font->GetColorTranslation(newcolor, &color);
				parms.color = PalEntry(colorparm.a, (color.r * colorparm.r) / 255, (color.g * colorparm.g) / 255, (color.b * colorparm.b) / 255);
				currentcolor = newcolor;
			}
			continue;
		}

		if (c == '\n')
		{
			cx = x;
			cy += parms.celly;
			continue;
		}

		if (NULL != (pic = font->GetChar(c, currentcolor, &w)))
		{
			// if palette translation is used, font colors will be ignored.
			if (!palettetrans) parms.TranslationId = trans;
			SetTextureParms(drawer, &parms, pic, cx, cy);
			if (parms.cellx)
			{
				w = parms.cellx;
				parms.destwidth = parms.cellx;
				parms.destheight = parms.celly;
			}
			if (parms.monospace == EMonospacing::CellLeft)
				parms.left = 0;
			else if (parms.monospace == EMonospacing::CellCenter)
				parms.left = w / 2.;
			else if (parms.monospace == EMonospacing::CellRight)
				parms.left = w;

			drawer->AddTexture(pic, parms);
		}
		if (parms.monospace == EMonospacing::Off)
		{
			cx += (w + kerning + parms.spacing) * scalex;
		}
		else
		{
			cx += (parms.spacing) * scalex;
		}

	}
}


// For now the 'drawer' parameter is a placeholder - this should be the way to handle it later to allow different drawers.
void DrawText(F2DDrawer *drawer, FFont* font, int normalcolor, double x, double y, const char* string, int tag_first, ...)
{
	Va_List tags;
	DrawParms parms;

	if (font == NULL || string == NULL)
		return;

	va_start(tags.list, tag_first);
	bool res = ParseDrawTextureTags(drawer, nullptr, 0, 0, tag_first, tags, &parms, DrawTexture_Text);
	va_end(tags.list);
	if (!res)
	{
		return;
	}

	const char *txt = (parms.localize && string[0] == '$') ? GStrings.GetString(string + 1) : string;
	DrawTextCommon<uint8_t>(drawer, font, normalcolor, x, y, (const uint8_t*)string, parms);
}


void DrawText(F2DDrawer *drawer, FFont* font, int normalcolor, double x, double y, const char32_t* string, int tag_first, ...)
{
	Va_List tags;
	DrawParms parms;

	if (font == NULL || string == NULL)
		return;

	va_start(tags.list, tag_first);
	bool res = ParseDrawTextureTags(drawer, nullptr, 0, 0, tag_first, tags, &parms, DrawTexture_Text);
	va_end(tags.list);
	if (!res)
	{
		return;
	}
	// [Gutawer] right now nothing needs the char32_t version to have localisation support, and i don't know how to do it
	assert(parms.localize == false);
	DrawTextCommon(drawer, font, normalcolor, x, y, string, parms);
}


void DrawText(F2DDrawer *drawer, FFont *font, int normalcolor, double x, double y, const FString& string, VMVa_List &args)
{
	DrawParms parms;

	if (font == NULL)
		return;

	uint32_t tag = ListGetInt(args);
	bool res = ParseDrawTextureTags(drawer, nullptr, 0, 0, tag, args, &parms, DrawTexture_Text, ~0u, 0.0, true);
	if (!res)
	{
		return;
	}
	const char *txt = (parms.localize && string.Len() >= 2 && string[0] == '$') ? GStrings.GetString(string.GetChars() + 1) : string.GetChars();

	DrawTextCommon(drawer, font, normalcolor, x, y, (uint8_t*)txt, parms);
}

DEFINE_ACTION_FUNCTION(_Screen, DrawText)
{
	PARAM_PROLOGUE;
	PARAM_POINTER_NOT_NULL(font, FFont);
	PARAM_INT(cr);
	PARAM_FLOAT(x);
	PARAM_FLOAT(y);
	PARAM_STRING(chr);

	PARAM_VA_POINTER(va_reginfo)	// Get the hidden type information array

	if (!twod->HasBegun2D()) ThrowAbortException(X_OTHER, "Attempt to draw to screen outside a draw function");
	VMVa_List args = { param + 5, 0, numparam - 6, va_reginfo + 5 };
	DrawText(twod, font, cr, x, y, chr, args);
	return 0;
}


DEFINE_ACTION_FUNCTION(FCanvas, DrawText)
{
	PARAM_SELF_PROLOGUE(FCanvas);
	PARAM_POINTER_NOT_NULL(font, FFont);
	PARAM_INT(cr);
	PARAM_FLOAT(x);
	PARAM_FLOAT(y);
	PARAM_STRING(chr);

	PARAM_VA_POINTER(va_reginfo)	// Get the hidden type information array

	VMVa_List args = { param + 6, 0, numparam - 7, va_reginfo + 6 };
	DrawText(&self->Drawer, font, cr, x, y, chr, args);
	self->Tex->NeedUpdate();
	return 0;
}
