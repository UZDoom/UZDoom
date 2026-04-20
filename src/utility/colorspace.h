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

namespace Color {

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

	Color(ColorSpace t, ColorP v1 = 0, ColorP v2 = 0, ColorP v3 = 0) : type(t)
	{
		rgb.r = v1; rgb.g = v2; rgb.b = v3;
	}

	friend Color rgb(ColorP r, ColorP g, ColorP b);
	friend Color rgb(const Color& c);
	friend void _2rgb(Color& c);
	friend void rgb2oklab(Color& c);
	friend void rgb2oklch(Color& c);

	friend Color oklch(ColorP L, ColorP c, ColorP h);
	friend Color oklch(const Color& c);
	friend void _2oklch(Color& c);
	friend void oklch2rgb(Color& c);
	friend void oklch2oklab(Color& c);

	friend Color oklab(ColorP L, ColorP a, ColorP b);
	friend Color oklab(const Color& c);
	friend void _2oklab(Color& c);
	friend void oklab2rgb(Color& c);
	friend void oklab2oklch(Color& c);

	friend Color mix(const Color& a, const Color& b, ColorP mix);
} Color;

Color rgb(ColorP r, ColorP g, ColorP b);
Color rgb(const Color& c);
void _2rbg(Color& c);
void rgb2oklab(Color& c);
void rgb2oklch(Color& c);

Color oklch(ColorP L, ColorP c, ColorP h);
Color oklch(const Color& c);
void _2oklch(Color& c);
void oklch2rgb(Color& lch);
void oklch2oklab(Color& lch);

Color oklab(ColorP L, ColorP a, ColorP b);
Color oklab(const Color& c);
void _2oklab(Color& c);
void oklab2rgb(Color& lab);
void oklab2oklch(Color& lab);

Color mix(const Color& a, const Color& b, ColorP mix);

}
