#include "Trex/Charset.hpp"
#include <stdexcept>

namespace Trex
{
	Charset::Charset(uint32_t first, uint32_t last)
		: Charset(std::span<const Range>({{first,last}})) {}

	Charset::Charset(const std::vector<Range>& codepointRanges)
		: Charset(std::span<const Range>(codepointRanges)) {}

	Charset::Charset(const std::span<const std::pair<uint32_t, uint32_t>> codepointRanges)
	{
		for (const auto& [first, last] : codepointRanges)
		{
			if (first > last)
			{
				throw std::runtime_error("Error: invalid charset range");
			}
			for (uint32_t codepoint = first; codepoint <= last; codepoint++)
			{
				m_Charset.insert(codepoint);
			}
		}
	}
}