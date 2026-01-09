#pragma once
#include <utility>
#include <set>
#include <vector>
#include <span>
#include <cstdint>


namespace Trex
{
	class Charset
	{
	public:
		using Range = std::pair<uint32_t, uint32_t>;

		explicit Charset() = default;
		explicit Charset(uint32_t first, uint32_t last);
		explicit Charset(const std::vector<Range>& codepointRanges);
		explicit Charset(std::span<const Range> codepointRanges);

		Charset(Charset&&) = default;
		Charset(const Charset&) = default;
		Charset& operator=(const Charset&) = default;

		static Charset Full()
		{
			Charset charset{};
			charset.m_AllCodepoints = true;
			return charset;
		}
		static Charset Ascii() { return Charset( 0, 127 ); };

		void AddCodepoint(const uint32_t codepoint) { m_Charset.insert(codepoint); }
		size_t Size() const { return m_Charset.size(); }
		const std::set<uint32_t> Codepoints() const { return m_Charset; }
		bool IsFull() const { return m_AllCodepoints; }

		auto begin() const { return m_Charset.begin(); }
		auto end() const { return m_Charset.end(); }

	private:
		std::set<uint32_t> m_Charset = {};
		bool m_AllCodepoints = false;
	};
}