/*
** m_crc32.h
** Simple interface to zlib's CRC table
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#include <array>
#include <cstdint>
#include <miniz.h>
#include <stdint.h>
#include <string_view>

inline constexpr std::array<uint32_t, 256> generate_crc32_table(uint32_t polynomial)
{
	std::array<uint32_t, 256> table{};
	for (auto i = 0; i < 256; i++)
	{
		uint32_t remainder = i;
		for (auto j = 0; j < 8; j++)
		{
			remainder = (remainder & 1) ? (remainder >> 1) ^ polynomial : (remainder >> 1);
		}
		table[i] = remainder;
	}
	return table;
}

/*
// VS19 doesn't like this
template <auto Transformer = nullptr>
inline constexpr uint32_t CalcCRC32(std::string_view data)
{
	uint32_t crc = 0xFFFFFFFF;
	for (uint8_t c : data)
	{
		uint8_t v;
		if constexpr(nullptr == Transformer) { v = c; }
		else { v = Transformer(c); }
		crc = (crc >> 8) ^ crc32_table[(crc ^ v) & 0xFF];
	}
	return crc ^ 0xFFFFFFFF;
}
*/

inline constexpr auto crc32_table = generate_crc32_table(0xEDB88320);
inline constexpr uint32_t CalcCRC32(std::string_view data)
{
	uint32_t crc = 0xFFFFFFFF;
	for (uint8_t c : data)
	{
		uint8_t v = c;
		crc = (crc >> 8) ^ crc32_table[(crc ^ v) & 0xFF];
	}
	return crc ^ 0xFFFFFFFF;
}

template<typename F>
inline constexpr uint32_t CalcCRC32(std::string_view data, F && Transformer)
{
	uint32_t crc = 0xFFFFFFFF;
	for (uint8_t c : data)
	{
		uint8_t v = Transformer(c);
		crc = (crc >> 8) ^ crc32_table[(crc ^ v) & 0xFF];
	}
	return crc ^ 0xFFFFFFFF;
}

inline uint32_t CalcCRC32 (const uint8_t *buf, unsigned int len)
{
	return crc32 (0, buf, len);
}
inline uint32_t AddCRC32 (uint32_t crc, const uint8_t *buf, unsigned int len)
{
	return crc32 (crc, buf, len);
}
inline uint32_t CRC1 (uint32_t crc, const uint8_t c, const uint32_t *crcTable)
{
	return crcTable[(crc & 0xff) ^ c] ^ (crc >> 8);
}

inline uint32_t Bcrc32(const void* data, int length, uint32_t crc)
{
	return crc32(crc, (const Bytef*)data, length);
}
