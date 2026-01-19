/*
** themedata.cpp
**
** Loads and holds color data
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

#include <cmath>

#include "themedata.h"
#include "basics.h"
#include "zwidget/core/colorf.h"

Colorf Theme::accent;
ThemeData Theme::dark = {};
ThemeData Theme::light = {};
ThemeData* Theme::theme = nullptr;
Mode Theme::mode;

void Theme::initilize(Mode mode)
{
	if (Theme::theme) return;

	setMode(mode);
	ThemeData *t;

	// hard-coded nice fallback
	Theme::accent = Colorf::fromRgb(0x237887);
	t = &Theme::light;
	t->main.bg   = Colorf::fromRgb(0xf1eee2);
	t->main.fg   = Colorf::fromRgb(0x000000);
	t->header.bg = Colorf::fromRgb(0xf5f4ef);
	t->header.fg = Colorf::fromRgb(0x000000);
	t->button.bg = Colorf::fromRgb(0xefe8cd);
	t->button.fg = Colorf::fromRgb(0x000000);
	t->hover.bg  = Colorf::fromRgb(0xa4c2e9);
	t->hover.fg  = Colorf::fromRgb(0x000000);
	t->click.bg  = Colorf::fromRgb(0x7ca2e9);
	t->click.fg  = Colorf::fromRgb(0x000000);
	t->border.bg = Colorf::fromRgb(0x586e75);
	t->border.fg = Colorf::fromRgb(0xbdb8a7);
	t = &Theme::dark;
	t->main.bg   = Colorf::fromRgb(0x002033);
	t->main.fg   = Colorf::fromRgb(0xffffff);
	t->header.bg = Colorf::fromRgb(0x001017);
	t->header.fg = Colorf::fromRgb(0xffffff);
	t->button.bg = Colorf::fromRgb(0x003747);
	t->button.fg = Colorf::fromRgb(0xffffff);
	t->hover.bg  = Colorf::fromRgb(0xa4c2e9);
	t->hover.fg  = Colorf::fromRgb(0x000000);
	t->click.bg  = Colorf::fromRgb(0x7ca2e9);
	t->click.fg  = Colorf::fromRgb(0x000000);
	t->border.bg = Colorf::fromRgb(0x536078);
	t->border.fg = Colorf::fromRgb(0x2b3e5b);

	// TODO: load from file
}

typedef float ColorP;
typedef union ColorSpace {
	struct rgb { ColorP r; ColorP g; ColorP b; } rgb;
	struct lab { ColorP L; ColorP a; ColorP b; } lab;
	struct lch { ColorP L; ColorP c; ColorP h; } lch;
} ColorSpace;

void oklch2oklab(ColorSpace& lch)
{
	auto c = lch.lch.c, h = lch.lch.h;
	lch.lab.a = isnan(h) ? 0 : c * cos(h * M_PI / 180.f);
	lch.lab.b = isnan(h) ? 0 : c * sin(h * M_PI / 180.f);
}

void oklab2oklch(ColorSpace& lab)
{
	auto a = lab.lab.a, b = lab.lab.b;
	lab.lch.c = sqrt(a*a + b*b);
	if (lab.lch.c < 0.0001f) {
		lab.lch.h = NAN;
	} else {
		lab.lch.h = atan2(b, a) * 180.0f / M_PI;
		lab.lch.h = lab.lch.h >= 0
			? lab.lch.h
			: lab.lch.h + 360.0f;
	}
}

inline ColorP gamma(ColorP c) { return clamp((c >= 0.0031308 ? 1.055 * pow(c, 1 / 2.4) - 0.055 : 12.92 * c), 0.0, 1.0); }
void oklab2rgb(ColorSpace& lab)
{
	auto L = lab.lab.L, a = lab.lab.a, b = lab.lab.b;
	auto l = pow((L + a * +0.3963377774 + b * +0.2158037573), 3);
	auto m = pow((L + a * -0.1055613458 + b * -0.0638541728), 3);
	auto s = pow((L + a * -0.0894841775 + b * -1.2914855480), 3);
	lab.rgb.r = gamma(l * +4.0767416621 + m * -3.3077115913 + s * +0.2309699292);
	lab.rgb.g = gamma(l * -1.2684380046 + m * +2.6097574011 + s * -0.3413193965);
	lab.rgb.b = gamma(l * -0.0041960863 + m * -0.7034186147 + s * +1.7076147010);
}

inline ColorP linear(ColorP c) { return c >= 0.04045 ? pow((c + 0.055) / 1.055, 2.4) : c / 12.92; };
void rgb2oklab(ColorSpace& c)
{
	auto r = linear(c.rgb.r), g = linear(c.rgb.g), b = linear(c.rgb.b);
	auto l = cbrt(0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b);
	auto m = cbrt(0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b);
	auto s = cbrt(0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b);
	c.lab.L = l * +0.2104542553 + m * +0.7936177850 + s * -0.0040720468;
	c.lab.a = l * +1.9779984951 + m * -2.4285922050 + s * +0.4505937099;
	c.lab.b = l * +0.0259040371 + m * +0.7827717662 + s * -0.8086757660;
}

ColorP alerp(ColorP a, ColorP b, float t) {
	if (isnan(a)) return isnan(b)? 0: b;
	if (isnan(b)) return isnan(a)? 0: a;
	ColorP delta = fmod(b - a, 360.0f);
	if (delta > 180.0f) delta -= 360.0f;
	if (delta < -180.0f) delta += 360.0f;
	return fmod(a + delta * t + 360.0f, 360.0f);
}

Colorf Theme::mix(const ColorLayers& color, float mix)
{
	if (mix <= 0) return color.bg;
	if (mix >= 1) return color.fg;

	ColorSpace a, b;
	a.rgb = { color.bg.r, color.bg.g, color.bg.b };
	b.rgb = { color.fg.r, color.fg.g, color.fg.b };
	rgb2oklab(a);
	rgb2oklab(b);
	oklab2oklch(a);
	oklab2oklch(b);
	a.lch.L = lerp(a.lch.L, b.lch.L, mix);
	a.lch.c = lerp(a.lch.c, b.lch.c, mix);
	a.lch.h = alerp(a.lch.h, b.lch.h, mix);
	oklch2oklab(a);
	oklab2rgb(a);

	return { a.rgb.r, a.rgb.g, a.rgb.b };
}

Colorf Theme::getAccent() { return { Theme::accent }; }

Colorf Theme::getMain  (float mix) { return Theme::mix(Theme::theme->main,   mix); }
Colorf Theme::getHeader(float mix) { return Theme::mix(Theme::theme->header, mix); }
Colorf Theme::getButton(float mix) { return Theme::mix(Theme::theme->button, mix); }
Colorf Theme::getHover (float mix) { return Theme::mix(Theme::theme->hover,  mix); }
Colorf Theme::getClick (float mix) { return Theme::mix(Theme::theme->click,  mix); }
Colorf Theme::getBorder(float mix) { return Theme::mix(Theme::theme->border, mix); }
