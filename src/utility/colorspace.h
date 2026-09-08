/*
** colorspace.h
**
** Convert between colorspaces
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <cstdint>
#include <string_view>

namespace Color {

consteval int str(std::string_view s)
{
	if ((s.length() != 7 && s.length() != 4) || s[0] != '#') throw "Not a color";
	s = s.substr(1);
	int rgb = 0;
    for (int i = 0, b = 0; i < 6; i++, b = i/(6/s.length())) {
		rgb <<= 4;
		if (s[b] >= '0' && s[b] <= '9') rgb |= (s[b] - '0');
		else if (s[b] >= 'a' && s[b] <= 'f') rgb |= (s[b] - 'a' + 10);
		else if (s[b] >= 'A' && s[b] <= 'F') rgb |= (s[b] - 'A' + 10);
		else throw "Bad byte";
	}
	return rgb;
}

#define T(a, b, c) \
	static_assert(Color::str(a) == Color::str(b), ""); \
	static_assert(Color::str(a) == c, "");
T("#000", "#000000", 0x000000);
T("#f00", "#ff0000", 0xff0000);
T("#0f0", "#00ff00", 0x00ff00);
T("#00f", "#0000ff", 0x0000ff);
T("#ff0", "#ffff00", 0xffff00);
T("#0ff", "#00ffff", 0x00ffff);
T("#f0f", "#ff00ff", 0xff00ff);
T("#fff", "#ffffff", 0xffffff);
static_assert(str("#7f7f7f") == 0x7f7f7f, "");
#undef T

enum ColorSpace {
	SRGB, OKLAB, OKLCH
};

typedef float ColorP;

typedef struct Color {
	union {
		struct { ColorP r, g, b; } rgb;
		struct { ColorP L, a, b; } lab;
		struct { ColorP L, c, h; } lch;
	};

private:
	ColorSpace type;

	constexpr Color(ColorSpace t, ColorP v1 = 0, ColorP v2 = 0, ColorP v3 = 0) : type(t)
	{
		rgb.r = v1; rgb.g = v2; rgb.b = v3;
	}

	friend constexpr Color rgb(ColorP r, ColorP g, ColorP b);
	friend Color rgb(const Color& c);
	friend void _2rgb(Color& c);
	friend void rgb2oklab(Color& c);
	friend void rgb2oklch(Color& c);

	friend constexpr Color oklch(ColorP L, ColorP c, ColorP h);
	friend Color oklch(const Color& c);
	friend void _2oklch(Color& c);
	friend void oklch2rgb(Color& c);
	friend void oklch2oklab(Color& c);

	friend constexpr Color oklab(ColorP L, ColorP a, ColorP b);
	friend Color oklab(const Color& c);
	friend void _2oklab(Color& c);
	friend void oklab2rgb(Color& c);
	friend void oklab2oklch(Color& c);

	friend Color mix(const Color& a, const Color& b, ColorP mix);
} Color;

constexpr Color rgb(ColorP r, ColorP g, ColorP b)
{
	return { SRGB, r, g, b };
}
Color rgb(const Color& c);
void _2rbg(Color& c);
void rgb2oklab(Color& c);
void rgb2oklch(Color& c);

constexpr Color oklch(ColorP L, ColorP c, ColorP h)
{
	return { OKLCH, L, c, h };
}
Color oklch(const Color& c);
void _2oklch(Color& c);
void oklch2rgb(Color& lch);
void oklch2oklab(Color& lch);

constexpr Color oklab(ColorP L, ColorP a, ColorP b)
{
	return { OKLAB, L, a, b };
}
Color oklab(const Color& c);
void _2oklab(Color& c);
void oklab2rgb(Color& lab);
void oklab2oklch(Color& lab);

Color mix(const Color& a, const Color& b, ColorP mix);

constexpr Color rgb(uint32_t rgb24)
{
	ColorP r = ((rgb24>>16)&0xff)/255.0f;
	ColorP g = ((rgb24>>8 )&0xff)/255.0f;
	ColorP b = ((rgb24    )&0xff)/255.0f;
	return rgb(r, g, b);
}
consteval Color rgb(std::string_view s)
{
	return rgb(str(s));
}
uint32_t rgb24(const Color& c);

}
