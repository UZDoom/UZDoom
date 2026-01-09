#include "Trex/TextShaper.hpp"
#include "hb.h"
#include "hb-ft.h"
#include <limits>

namespace Trex
{

	TextShaper::TextShaper(const Trex::Atlas& atlas)
		: m_Glyphs(atlas.GetGlyphs()),
		  m_AtlasFont(atlas.GetFont()),
		  m_Buffer(hb_buffer_create()),
		  m_Font(hb_ft_font_create_referenced(m_AtlasFont->face))
	{
	}

	TextShaper::~TextShaper()
	{
		hb_buffer_destroy(m_Buffer);
		hb_font_destroy(m_Font);
	}

	ShapedGlyphs TextShaper::ShapeUtf8(const std::span<const char> text)
	{
		ResetBuffer();
		hb_buffer_add_utf8(m_Buffer, text.data(), (int)text.size(), 0, (int)text.size());
		hb_shape(m_Font, m_Buffer, nullptr, 0);

		return GetShapedGlyphs();
	}

	ShapedGlyphs TextShaper::ShapeUtf32(const std::span<const char32_t> text)
	{
		// Unfortunately casting char32_t* to uint32_t* violates the strict aliasing rule.
		// char32_t and uint32_t are not the same type, even though they are both 32 bits wide.
		std::vector<uint32_t> codepoints(text.begin(), text.end());
		return ShapeUnicode(codepoints);
	}

	ShapedGlyphs TextShaper::ShapeUnicode(const std::span<const uint32_t> codepoints)
	{
		ResetBuffer();
		hb_buffer_add_codepoints(m_Buffer, codepoints.data(), (int)codepoints.size(), 0, (int)codepoints.size());
		hb_shape(m_Font, m_Buffer, nullptr, 0);

		return GetShapedGlyphs();
	}

	FontMetrics TextShaper::GetFontMetrics() const
	{
		return m_AtlasFont->GetMetrics();
	}

	TextMeasurement TextShaper::Measure(const Trex::ShapedGlyphs& glyphs)
	{
		if (glyphs.empty())
			return TextMeasurement{}; // Filled with zeros

		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float cursorX = 0.0f;
		float cursorY = 0.0f;
		for (const auto& glyph : glyphs)
		{
			float x = cursorX + glyph.xOffset + (float)glyph.info.bearingX;
			float y = cursorY + glyph.yOffset - (float)glyph.info.bearingY;

			minX = std::min(minX, x);
			maxX = std::max(maxX, x + (float)glyph.info.width);
			minY = std::min(minY, y);
			maxY = std::max(maxY, y + (float)glyph.info.height);

			cursorX += glyph.xAdvance;
			cursorY += glyph.yAdvance;
		}

		return TextMeasurement {
				.width = maxX - minX,
				.height = maxY - minY,
				.xOffset = minX,
				.yOffset = minY,
				.xAdvance = cursorX,
				.yAdvance = cursorY
		};
	}

	Glyph TextShaper::GetAtlasGlyph(uint32_t index)
	{
		const auto& glyphs = m_Glyphs.Data();
		return glyphs.contains( index ) ? glyphs.at( index ) : m_Glyphs.GetUnknownGlyph();
	}

	ShapedGlyphs TextShaper::GetShapedGlyphs()
	{
		unsigned int glyphCount;
		hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(m_Buffer, &glyphCount);
		hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(m_Buffer, &glyphCount);

		ShapedGlyphs glyphs;
		glyphs.reserve(glyphCount);
		for (unsigned int i = 0; i < glyphCount; i++)
		{
			ShapedGlyph glyph = GetShapedGlyph(glyphInfo[i], glyphPos[i]);
			glyphs.push_back(glyph);
		}

		return glyphs;
	}

	ShapedGlyph TextShaper::GetShapedGlyph(const hb_glyph_info_t& glyphInfo, const hb_glyph_position_t& glyphPos)
	{
		unsigned int glyphIndex = glyphInfo.codepoint; // after shaping codepoint becomes glyph index
		ShapedGlyph glyph{};
		glyph.info = GetAtlasGlyph(glyphIndex);
		glyph.xOffset = static_cast<float>(glyphPos.x_offset) / 64.0f;
		glyph.yOffset = static_cast<float>(glyphPos.y_offset) / 64.0f;
		glyph.xAdvance = static_cast<float>(glyphPos.x_advance) / 64.0f;
		glyph.yAdvance = static_cast<float>(glyphPos.y_advance) / 64.0f;
		return glyph;
	}

	void TextShaper::ResetBuffer()
	{
		hb_buffer_reset(m_Buffer);
		hb_buffer_set_direction(m_Buffer, HB_DIRECTION_LTR);
		hb_buffer_set_script(m_Buffer, HB_SCRIPT_COMMON);
		hb_buffer_set_language(m_Buffer, hb_language_from_string("pl", -1));
	}
}