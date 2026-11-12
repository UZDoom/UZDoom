#pragma once
#include "Atlas.hpp"
#include <vector>
#include <span>

struct hb_glyph_info_t;
struct hb_glyph_position_t;
struct hb_buffer_t;
struct hb_font_t;

namespace Trex
{
	struct ShapedGlyph
	{
		float xOffset;
		float yOffset;
		float xAdvance;
		float yAdvance;

		Glyph info;
	};

	using ShapedGlyphs = std::vector<ShapedGlyph>;

	struct TextMeasurement
	{
		float width, height; // Width and height of the text. Measured from the top-left corner (offset)
		float xOffset, yOffset; // Offset from baseline origin to left/top
		float xAdvance, yAdvance; // Advance from baseline origin to the end of the text (including trailing advance)
	};

	class TextShaper
	{
	public:
		explicit TextShaper(const Atlas& atlas);
		~TextShaper();

		ShapedGlyphs ShapeAscii(const std::span<const char> text)
		{
			// UTF-8 encoding of ASCII characters is the same as ASCII encoding.
			return ShapeUtf8(text);
		}
		ShapedGlyphs ShapeUtf8(std::span<const char> text);
		ShapedGlyphs ShapeUtf32(std::span<const char32_t> text);
		ShapedGlyphs ShapeUnicode(std::span<const uint32_t> codepoints);

		FontMetrics GetFontMetrics() const;

		static TextMeasurement Measure(const ShapedGlyphs&);

	private:
		Glyph GetAtlasGlyph(uint32_t glyphIndex);
		ShapedGlyphs GetShapedGlyphs();
		ShapedGlyph GetShapedGlyph(const hb_glyph_info_t& glyphInfo, const hb_glyph_position_t& glyphPos);
		void ResetBuffer();

		Atlas::Glyphs m_Glyphs;
		std::shared_ptr<const Font> m_AtlasFont;

		hb_buffer_t* m_Buffer;
		hb_font_t* m_Font;
	};

}