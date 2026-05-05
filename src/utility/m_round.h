/*
** m_round.h
**
** Float to Int routines
**
**---------------------------------------------------------------------------
*
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

inline static int32_t RoundUp(double val)
{
	return static_cast<int32_t>(std::ceil(val));
}

inline static int32_t RoundDown(double val)
{
	return static_cast<int32_t>(std::floor(val));
}

inline static int32_t RoundToZero(double val)
{
	return static_cast<int32_t>(val);
}

inline static int32_t RoundFromZero(double val)
{
	return static_cast<int32_t>((val >= 0)? std::ceil(val): std::floor(val));
}

inline static int32_t RoundHalfUp(double val)
{
	return static_cast<int32_t>(std::floor(val + 0.5));
}

inline static int32_t RoundHalfDown(double val)
{
	return static_cast<int32_t>(std::ceil(val - 0.5));
}

inline static int32_t RoundHalfEven(double val)
{
#if 1
	static_assert(std::numeric_limits<double>::is_iec559, "needs ieee-754");
	static_assert(std::numeric_limits<double>::round_style == std::round_to_nearest,  "needs round-to-nearest");
	return static_cast<int32_t>(std::llrint(val)); // this is potentially inconsistent on different platforms
	                                               // but it's no more inconsistent than xs_Float was
	                                               // implementation below should be slower but consistent on all setups
#else
	double r = std::floor(val + 0.5);
	bool mid = ((r - val) == 0.5) && (std::fmod(r, 2.0) != 0.0);
	return static_cast<int32_t>((long)(mid ? r - 1.0 : r));
#endif
}

inline static int32_t RoundHalfToZero(double val)
{
	return static_cast<int32_t>((val >= 0)? std::ceil(val - 0.5): std::floor(val + 0.5));
}

inline static int32_t RoundHalfFromZero(double val)
{
	return static_cast<int32_t>((val >= 0)? std::floor(val + 0.5): std::ceil(val - 0.5));
}
