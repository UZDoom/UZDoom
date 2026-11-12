#pragma once
#include <cstdint>
#include <span>
#include <vector>
#include <variant>

struct FT_FaceRec_;

namespace Trex
{
	struct Pixels { int value; };
	struct Points { int value; };
	using FontSize = std::variant<Pixels, Points>;

	struct FontMetrics
	{
		int ascender;  // Distance from baseline to top of glyph (positive value)
		int descender; // Distance from baseline to bottom of glyph (negative value)
		int height;    // Distance from baseline to the next line's baseline (ascender - descender + linegap)
	};

	class Font
	{
	public:
		explicit Font(const char* path);
		explicit Font(std::span<const uint8_t> data);
		Font(Font&&) noexcept;
		~Font();

		void SetSize(const FontSize& size);
		uint32_t GetGlyphIndex(uint32_t codepoint) const;

		FontMetrics GetMetrics() const;

		FT_FaceRec_* face = nullptr;

	private:
		void SetSizeInPixels(Pixels size);
		void SetSizeInPoints(Points size);


		std::vector<uint8_t> fontData = {};
	};
}