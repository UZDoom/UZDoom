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

#include <zwidget/core/colorf.h>
#include <zwidget/core/resourcedata.h>

#include "sc_man.h"
#include "themedata.h"
#include "utility/colorspace.h"

Colorf Theme::accent;
ThemeData Theme::dark = {};
ThemeData Theme::light = {};
ThemeData* Theme::theme = nullptr;
Mode Theme::mode;

enum ThemeCommands
{
	THEME_LIGHT,
	THEME_DARK,
	THEME_ACCENT,
	THEME_MAIN,
	THEME_HEADER,
	THEME_BUTTON,
	THEME_HOVER,
	THEME_CLICK,
	THEME_BORDER,
};

static const char *ThemeCommandStrings[] =
{
	"[light]",
	"[dark]",
	"accent",
	"main",
	"header",
	"button",
	"hover",
	"click",
	"border",
	nullptr
};

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

	FScanner sc;
	auto file = "ui/theme.txt";
	auto buffer = LoadWidgetData(file);
	sc.OpenMem(file, buffer);

	auto hex = [](FScanner &sc) {
		sc.MustGetString();
		if (sc.String[0] != '#') return -1;
		char *end;
		int rgb = std::strtol(sc.String + 1, &end, 16);
		auto len = end - (sc.String + 1);
		if (len == 3)
		{
			rgb = (rgb>>8&0xF)<<16 | (rgb>>4&0xF)<<8 | (rgb>>0&0xF)<<0;
			rgb |= rgb<<4;
		}
		else if (len != 6) rgb = -1;
		return rgb;
	};

	auto pair = [=](FScanner &sc, ColorLayers &layers) {
		layers.bg = Colorf::fromRgb(hex(sc));
		layers.fg = Colorf::fromRgb(hex(sc));
	};

	t = nullptr;
	while (sc.GetString ())
	{
		if (sc.String[0] == '/' && sc.String[1] == '/') continue; // GetString is guaranteed to be at least 1 char + null, right?

		switch (sc.MatchString(ThemeCommandStrings))
		{
		case THEME_LIGHT:
			t = &Theme::light;
			break;
		case THEME_DARK:
			t = &Theme::dark;
			break;
		case THEME_ACCENT:
			Theme::accent = Colorf::fromRgb(hex(sc));
			break;
		case THEME_MAIN:
			if (t) pair(sc, t->main);
			break;
		case THEME_HEADER:
			if (t) pair(sc, t->header);
			break;
		case THEME_BUTTON:
			if (t) pair(sc, t->button);
			break;
		case THEME_HOVER:
			if (t) pair(sc, t->hover);
			break;
		case THEME_CLICK:
			if (t) pair(sc, t->click);
			break;
		case THEME_BORDER:
			if (t) pair(sc, t->border);
			break;
		}
	}
}

Colorf Theme::mix(const ColorLayers& color, float mix)
{
	auto a = Color::rgb(color.bg.r, color.bg.g, color.bg.b);
	auto b = Color::rgb(color.fg.r, color.fg.g, color.fg.b);
	auto c = Color::mix(a, b, mix).rgb;
	return { c.r, c.g, c.b };
}

Colorf Theme::getAccent() { return { Theme::accent }; }

Colorf Theme::getMain  (float mix) { return Theme::mix(Theme::theme->main,   mix); }
Colorf Theme::getHeader(float mix) { return Theme::mix(Theme::theme->header, mix); }
Colorf Theme::getButton(float mix) { return Theme::mix(Theme::theme->button, mix); }
Colorf Theme::getHover (float mix) { return Theme::mix(Theme::theme->hover,  mix); }
Colorf Theme::getClick (float mix) { return Theme::mix(Theme::theme->click,  mix); }
Colorf Theme::getBorder(float mix) { return Theme::mix(Theme::theme->border, mix); }
