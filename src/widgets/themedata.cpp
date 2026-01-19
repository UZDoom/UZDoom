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
#include "zwidget/core/colorf.h"

ThemeData Theme::dark = {};
ThemeData Theme::light = {};
ThemeData* Theme::theme = nullptr;
Mode Theme::mode;

void Theme::initilize(Mode mode)
{
	if (Theme::theme) return;

	setMode(mode);
	ThemeData *t;

	// basic fallback
	Theme::light.main.bg = Colorf::fromRgb(0xffffff);
	Theme::light.main.fg = Colorf::fromRgb(0x000000);
	Theme::light.border.fg = Colorf::fromRgb(0x7f7f7f);
	t = &Theme::light;
	t->main.bg = t->header.bg = t->button.bg = t->hover.bg = t->click.bg = Theme::light.main.bg;
	t->main.fg = t->header.fg = t->button.fg = t->hover.fg = t->click.fg = Theme::light.main.fg;
	t->border.bg = t->border.fg = Theme::light.border.fg;
	t = &Theme::dark;
	t->main.bg = t->header.bg = t->button.bg = t->hover.bg = t->click.bg = Theme::light.main.fg;
	t->main.fg = t->header.fg = t->button.fg = t->hover.fg = t->click.fg = Theme::light.main.bg;
	t->border.bg = t->border.fg = Theme::light.border.fg;

	// hard-coded nice fallback
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

Colorf Theme::mix(ColorLayers color, float mix)
{
	if (mix <= 0) return color.bg;
	if (mix >= 1) return color.fg;

	// TODO: better mix

	float r = std::lerp(color.bg.r, color.fg.r, mix);
	float g = std::lerp(color.bg.g, color.fg.g, mix);
	float b = std::lerp(color.bg.b, color.fg.b, mix);

	return { r, g, b };
}

Colorf Theme::getMain  (float mix) { return Theme::mix(Theme::theme->main,   mix); }
Colorf Theme::getHeader(float mix) { return Theme::mix(Theme::theme->header, mix); }
Colorf Theme::getButton(float mix) { return Theme::mix(Theme::theme->button, mix); }
Colorf Theme::getHover (float mix) { return Theme::mix(Theme::theme->hover,  mix); }
Colorf Theme::getClick (float mix) { return Theme::mix(Theme::theme->click,  mix); }
Colorf Theme::getBorder(float mix) { return Theme::mix(Theme::theme->border, mix); }
