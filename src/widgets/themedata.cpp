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
	t->main.bg   = Colorf::fromRgb(0xeee8d5);
	t->main.fg   = Colorf::fromRgb(0x000000);
	t->header.bg = Colorf::fromRgb(0xfdf6e3);
	t->header.fg = Colorf::fromRgb(0x000000);
	t->button.bg = Colorf::fromRgb(0xd7d2bf);
	t->button.fg = Colorf::fromRgb(0x000000);
	t->hover.bg  = Colorf::fromRgb(0xa4c2e9);
	t->hover.fg  = Colorf::fromRgb(0x000000);
	t->click.bg  = Colorf::fromRgb(0x7ca2e9);
	t->click.fg  = Colorf::fromRgb(0x000000);
	t->border.bg = Colorf::fromRgb(0x586e75);
	t->border.fg = Colorf::fromRgb(0xbdb8a7);
	t = &Theme::dark;
	t->main.bg   = Colorf::fromRgb(0x2a2a2a);
	t->main.fg   = Colorf::fromRgb(0xe2dfdb);
	t->header.bg = Colorf::fromRgb(0x212121);
	t->header.fg = Colorf::fromRgb(0xe2dfdb);
	t->button.bg = Colorf::fromRgb(0x444444);
	t->button.fg = Colorf::fromRgb(0xffffff);
	t->hover.bg  = Colorf::fromRgb(0xc83c00);
	t->hover.fg  = Colorf::fromRgb(0xffffff);
	t->click.bg  = Colorf::fromRgb(0xbbbbbb);
	t->click.fg  = Colorf::fromRgb(0x000000);
	t->border.bg = Colorf::fromRgb(0x646464);
	t->border.fg = Colorf::fromRgb(0x555555);

	// TODO: load from file
}

Colorf Theme::mix(ColorLayers color, float mix)
{
	// TODO: actually mix
	return mix>0.5? color.fg: color.bg;
}

Colorf Theme::getMain  (float mix) { return Theme::mix(Theme::theme->main,   mix); }
Colorf Theme::getHeader(float mix) { return Theme::mix(Theme::theme->header, mix); }
Colorf Theme::getButton(float mix) { return Theme::mix(Theme::theme->button, mix); }
Colorf Theme::getHover (float mix) { return Theme::mix(Theme::theme->hover,  mix); }
Colorf Theme::getClick (float mix) { return Theme::mix(Theme::theme->click,  mix); }
Colorf Theme::getBorder(float mix) { return Theme::mix(Theme::theme->border, mix); }
