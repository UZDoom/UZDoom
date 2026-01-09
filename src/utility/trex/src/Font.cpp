#include "Trex/Font.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include <iostream>

namespace Trex
{
	FT_Library GetFTLibrary()
	{
        static FT_Library library;

		if (library == nullptr)
		{
			if(FT_Init_FreeType(&library))
			{
				throw std::runtime_error("Error: could not initialize FreeType library");
			}
		}
		return library;
	}

	Font::Font(const char* path)
	{
		FT_Long faceIndex = 0; // Take the first face in the font file
		FT_Library library = GetFTLibrary();

		if(FT_New_Face(library, path, faceIndex, &face))
		{
			throw std::runtime_error("Error: could not load font");
		}

		SetSize(Points{ 12 }); // Default size
	}

	Font::Font(std::span<const uint8_t> data)
		: fontData(std::vector<uint8_t>(data.begin(), data.end()))
	{
		FT_Long faceIndex = 0; // Take the first face in the font file
		FT_Library library = GetFTLibrary();
        const auto fontDataBytes = reinterpret_cast<const FT_Byte*>(fontData.data());
        const auto fontDataSize = static_cast<long>(fontData.size());

		if(FT_New_Memory_Face(library, fontDataBytes, fontDataSize, faceIndex, &face))
		{
			throw std::runtime_error("Error: could not load font");
		}

		SetSize(Points{ 12 }); // Default size
	}

	Font::Font(Font&& other) noexcept
	{
		FT_Reference_Face(other.face);
		face = other.face;
		other.face = nullptr;
		fontData = std::move(other.fontData);
	}

	Font::~Font()
	{
		FT_Done_Face(face);
	}

	void Font::SetSize(const FontSize& size)
	{
		if (std::holds_alternative<Pixels>(size))
		{
			SetSizeInPixels(std::get<Pixels>(size));
		}
		else if (std::holds_alternative<Points>(size))
		{
			SetSizeInPoints(std::get<Points>(size));
		}
		else
		{
			throw std::runtime_error("Error: invalid font size type");
		}
	}

	void Font::SetSizeInPixels(Pixels size) // NOLINT(readability-make-member-function-const)
	{
		if( FT_HAS_FIXED_SIZES( face ) )
		{
			FT_Select_Size(face, 0); // Select the first bitmap strike (should be the only one)
			return;
		}

		FT_Error error = FT_Set_Pixel_Sizes(face, 0, size.value);
		if (error)
		{
			throw std::runtime_error("Error: could not set font size");
		}
	}

	void Font::SetSizeInPoints(Points size) // NOLINT(readability-make-member-function-const)
	{
		if( FT_HAS_FIXED_SIZES( face ) )
		{
			FT_Select_Size( face, 0 ); // Select the first bitmap strike (should be the only one)
			return;
		}

		FT_Error error = FT_Set_Char_Size(face, 0, size.value * 64, 0, 0); // 72 dpi used as default
		if (error)
		{
			throw std::runtime_error("Error: could not set font size");
		}
	}

	uint32_t Font::GetGlyphIndex(uint32_t codepoint) const
	{
		return FT_Get_Char_Index(face, codepoint);
	}

	FontMetrics Font::GetMetrics() const
	{
		FontMetrics metrics;
		metrics.ascender = face->size->metrics.ascender / 64;
		metrics.descender = face->size->metrics.descender / 64;
		metrics.height = face->size->metrics.height / 64;
		return metrics;
	}

}