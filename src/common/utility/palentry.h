/*
** palentry.h
**
** Palette and color utility functions
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#pragma once

#include <algorithm>
#include <stdint.h>

// Beware of windows.h :(
#ifdef max
#undef min
#undef max
#endif

struct PalEntry
{
	PalEntry() = default;
	PalEntry(const PalEntry&) = default;
	constexpr PalEntry (uint32_t argb) : d(argb) { }
	operator uint32_t () const { return d; }
	void SetRGB(PalEntry other)
	{
		d = other.d & 0xffffff;
	}
	PalEntry Modulate(PalEntry other) const
	{
		if (isWhite())
		{
			return other;
		}
		else if (other.isWhite())
		{
			return *this;
		}
		else
		{
			other.r = (r * other.r) / 255;
			other.g = (g * other.g) / 255;
			other.b = (b * other.b) / 255;
			return other;
		}
	}
	constexpr int Luminance() const 
	{
		return (r * 77 + g * 143 + b * 37) >> 8;
	}

	constexpr int Amplitude() const
	{
		return std::max(r, std::max(g, b));
	}

	constexpr void Decolorize()	// this for 'nocoloredspritelighting' and not the same as desaturation. The normal formula results in a value that's too dark.
	{
		int v = (r + g + b);
		r = g = b = ((255*3) + v + v) / 9;
	}
	constexpr bool isBlack() const
	{
		return (d & 0xffffff) == 0;
	}
	constexpr bool isWhite() const
	{
		return (d & 0xffffff) == 0xffffff;
	}
	PalEntry &operator= (const PalEntry &other) = default;
	constexpr PalEntry &operator= (uint32_t other) { d = other; return *this; }
	constexpr PalEntry InverseColor() const { PalEntry nc(a, 255 - r, 255 - g, 255 - b); return nc; }
#ifdef __BIG_ENDIAN__
	constexpr PalEntry (uint8_t ir, uint8_t ig, uint8_t ib) : a(0), r(ir), g(ig), b(ib) {}
	constexpr PalEntry (uint8_t ia, uint8_t ir, uint8_t ig, uint8_t ib) : a(ia), r(ir), g(ig), b(ib) {}
	union
	{
		struct
		{
			uint8_t a,r,g,b;
		};
		uint32_t d;
	};
#else
	constexpr PalEntry (uint8_t ir, uint8_t ig, uint8_t ib) : b(ib), g(ig), r(ir), a(0) {}
	constexpr PalEntry (uint8_t ia, uint8_t ir, uint8_t ig, uint8_t ib) : b(ib), g(ig), r(ir), a(ia) {}
	union
	{
		struct
		{
			uint8_t b,g,r,a;
		};
		uint32_t d;
	};
#endif
};

constexpr inline int Luminance(int r, int g, int b)
{
	return (r * 77 + g * 143 + b * 37) >> 8;
}

#define APART(c)			(((c)>>24)&0xff)
#define RPART(c)			(((c)>>16)&0xff)
#define GPART(c)			(((c)>>8)&0xff)
#define BPART(c)			((c)&0xff) 
#define MAKERGB(r,g,b)		uint32_t(((r)<<16)|((g)<<8)|(b))
#define MAKEARGB(a,r,g,b)	uint32_t(((a)<<24)|((r)<<16)|((g)<<8)|(b)) 
