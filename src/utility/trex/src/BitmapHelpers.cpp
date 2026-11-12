#include "Trex/BitmapHelpers.hpp"

namespace Trex
{
	std::vector<uint8_t> ConvertBitmapToGrayAlpha(const std::span<const uint8_t> input)
	{
		std::vector<uint8_t> output(input.size() * 2);
		for (size_t i = 0; i < input.size(); i++)
		{
			output[i * 2] = input[i];
			output[i * 2 + 1] = 255;
		}
		return output;
	}

	std::vector<uint8_t> ConvertBitmapToRGB(const std::span<const uint8_t> input)
	{
		std::vector<uint8_t> output(input.size() * 3);
		for (size_t i = 0; i < input.size(); i++)
		{
			output[i * 3] = input[i];
			output[i * 3 + 1] = input[i];
			output[i * 3 + 2] = input[i];
		}
		return output;
	}

	std::vector<uint8_t> ConvertBitmapToRGBA(const std::span<const uint8_t> input)
	{
		std::vector<uint8_t> output(input.size() * 4);
		for (size_t i = 0; i < input.size(); i++)
		{
			output[i * 4] = input[i];
			output[i * 4 + 1] = input[i];
			output[i * 4 + 2] = input[i];
			output[i * 4 + 3] = 255;
		}
		return output;
	}
}