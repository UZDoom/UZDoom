/*
** m_fixed.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#ifndef __M_FIXED__
#define __M_FIXED__

#include <stdlib.h>
#include <stdint.h>
#include "basics.h"


__forceinline constexpr int32_t MulScale(int32_t a, int32_t b, int32_t shift) { return (int32_t)(((int64_t)a * b) >> shift); }
__forceinline constexpr int32_t DMulScale(int32_t a, int32_t b, int32_t c, int32_t d, int32_t shift) { return (int32_t)(((int64_t)a * b + (int64_t)c * d) >> shift); }
__forceinline constexpr int32_t DivScale(int32_t a, int32_t b, int shift) { return (int32_t)(((int64_t)a << shift) / b); }
__forceinline constexpr int64_t DivScaleL(int64_t a, int64_t b, int shift) { return ((a << shift) / b); }

#include "xs_Float.h"

template<int b = 16>
constexpr fixed_t FloatToFixed(double f)
{
	return int(f * (1 << b));
}

constexpr fixed_t FloatToFixed(double f, int b)
{
	return int(f * (1 << b));
}

template<int b = 16>
inline constexpr fixed_t IntToFixed(int32_t f)
{
	return f << b;
}

template<int b = 16>
inline constexpr double FixedToFloat(fixed_t f)
{
	return f * (1. / (1 << b));
}

inline constexpr double FixedToFloat(fixed_t f, int b)
{
	return f * (1. / (1 << b));
}

template<int b = 16>
inline constexpr int32_t FixedToInt(fixed_t f)
{
	return (f + (1 << (b-1))) >> b;
}

inline unsigned FloatToAngle(double f)
{
	return xs_CRoundToInt((f)* (0x40000000 / 90.));
}

#define FLOAT2FIXED(f)		FloatToFixed(f)
#define FIXED2FLOAT(f)		float(FixedToFloat(f))
#define FIXED2DBL(f)		FixedToFloat(f)


#endif
