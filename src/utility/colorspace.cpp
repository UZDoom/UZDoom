/*
** colorspace.cpp
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

#include <cassert>
#include <cmath>

#include "basics.h"
#include "colorspace.h"

namespace Color {

constexpr ColorP N_180 = static_cast<ColorP>(180);
constexpr ColorP N_360 = static_cast<ColorP>(360);
constexpr ColorP N_PI = static_cast<ColorP>(M_PI);

#define CONVERT(color, from, to) \
	assert(color.type == from);  \
	color.type = to;

Color rgb(ColorP r, ColorP g, ColorP b)
{
	return { SRGB, r, g, b };
}

Color rgb(const Color& c)
{
	Color C {c};
	memcpy(&C, &c, sizeof(c));
	_2rgb(C);
	return C;
}

void _2rgb(Color& c)
{
	switch (c.type)
	{
	case OKLAB: oklab2rgb(c); break;
	case OKLCH: oklch2rgb(c); break;
	case SRGB: break;
	}
}

inline ColorP linear(ColorP c) { return c >= 0.04045 ? pow((c + 0.055) / 1.055, 2.4) : c / 12.92; };
void rgb2oklab(Color& rgb)
{
	CONVERT(rgb, SRGB, OKLAB);
	auto r = linear(rgb.rgb.r), g = linear(rgb.rgb.g), b = linear(rgb.rgb.b);
	auto l = cbrt(0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b);
	auto m = cbrt(0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b);
	auto s = cbrt(0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b);
	rgb.lab.L = l * +0.2104542553 + m * +0.7936177850 + s * -0.0040720468;
	rgb.lab.a = l * +1.9779984951 + m * -2.4285922050 + s * +0.4505937099;
	rgb.lab.b = l * +0.0259040371 + m * +0.7827717662 + s * -0.8086757660;
}

void rgb2oklch(Color& rgb)
{
	rgb2oklab(rgb);
	oklab2oklch(rgb);
}

Color oklch(ColorP L, ColorP c, ColorP h)
{
	return { OKLCH, L, c, h };
}

Color oklch(const Color& c)
{
	Color C {c};
	memcpy(&C, &c, sizeof(c));
	_2oklch(C);
	return C;
}

void _2oklch(Color& c)
{
	switch (c.type)
	{
	case OKLAB: oklab2oklch(c); break;
	case OKLCH: break;
	case SRGB: rgb2oklch(c); break;
	}
}

void oklch2rgb(Color& lch)
{
	oklch2oklab(lch);
	oklab2rgb(lch);
}

void oklch2oklab(Color& lch)
{
	CONVERT(lch, OKLCH, OKLAB);
	auto c = lch.lch.c, h = lch.lch.h;
	lch.lab.a = isnan(h) ? 0 : c * cos(h * N_PI / N_180);
	lch.lab.b = isnan(h) ? 0 : c * sin(h * N_PI / N_180);
}

Color oklab(ColorP L, ColorP a, ColorP b)
{
	return { OKLAB, L, a, b };
}

Color oklab(const Color& c)
{
	Color C {c};
	memcpy(&C, &c, sizeof(c));
	_2oklab(C);
	return C;
}

void _2oklab(Color& c)
{
	switch (c.type)
	{
	case OKLAB: break;
	case OKLCH: oklch2oklab(c); break;
	case SRGB: rgb2oklab(c); break;
	}
}

inline ColorP gamma(ColorP c) { return std::clamp((c >= 0.0031308 ? 1.055 * pow(c, 1 / 2.4) - 0.055 : 12.92 * c), 0.0, 1.0); }
void oklab2rgb(Color& lab)
{
	CONVERT(lab, OKLAB, SRGB);
	auto L = lab.lab.L, a = lab.lab.a, b = lab.lab.b;
	auto l = pow((L + a * +0.3963377774 + b * +0.2158037573), 3);
	auto m = pow((L + a * -0.1055613458 + b * -0.0638541728), 3);
	auto s = pow((L + a * -0.0894841775 + b * -1.2914855480), 3);
	lab.rgb.r = gamma(l * +4.0767416621 + m * -3.3077115913 + s * +0.2309699292);
	lab.rgb.g = gamma(l * -1.2684380046 + m * +2.6097574011 + s * -0.3413193965);
	lab.rgb.b = gamma(l * -0.0041960863 + m * -0.7034186147 + s * +1.7076147010);
}

void oklab2oklch(Color& lab)
{
	CONVERT(lab, OKLAB, OKLCH);
	auto a = lab.lab.a, b = lab.lab.b;
	lab.lch.c = sqrt(a*a + b*b);
	if (lab.lch.c < 0.0001f) {
		lab.lch.h = NAN;
	} else {
		lab.lch.h = atan2(b, a) * N_180 / N_PI;
		lab.lch.h = lab.lch.h >= 0
			? lab.lch.h
			: lab.lch.h + N_360;
	}
}

inline ColorP alerp(ColorP a, ColorP b, float t) {
	if (isnan(a)) return isnan(b)? 0: b;
	if (isnan(b)) return isnan(a)? 0: a;
	ColorP delta = fmod(b - a, N_360);
	if (delta > N_180) delta -= N_360;
	if (delta < -N_180) delta += N_360;
	return fmod(a + delta * t + N_360, N_360);
}

Color mix(const Color& a, const Color& b, ColorP mix)
{
	Color A {a};
	Color B {b};
	memcpy(&A, &a, sizeof(a));
	memcpy(&B, &b, sizeof(b));

	if (mix <= 0) return A;
	if (mix >= 1) return B;

	ColorSpace type = A.type;
	_2oklch(A);
	_2oklch(B);

	A.lch.L = std::lerp(A.lch.L, B.lch.L, mix);
	A.lch.c = std::lerp(A.lch.c, B.lch.c, mix);
	A.lch.h = alerp(A.lch.h, B.lch.h, mix);

	switch (type)
	{
	case SRGB: oklch2rgb(A); break;
	case OKLAB: oklch2oklab(A); break;
	case OKLCH: break;
	};

	return A;
}

#undef CONVERT

}
