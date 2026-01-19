/*
** themedata.h
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

#pragma once

#include <zwidget/core/colorf.h>

enum Mode { LIGHT, DARK };

constexpr auto COLOR_BACKGROUND = 0.0f;
constexpr auto COLOR_MIX = 0.5f;
constexpr auto COLOR_TEXT = 1.0f;
constexpr auto COLOR_LIGHT = 0.0f;
constexpr auto COLOR_HEAVY = 1.0f;

typedef struct { Colorf bg; Colorf fg; } ColorLayers;

typedef struct ThemeData
{
	ColorLayers main;
	ColorLayers header;
	ColorLayers button;
	ColorLayers hover;
	ColorLayers click;
	ColorLayers border;
} ThemeData;

class Theme
{
private:
	static ThemeData light;
	static ThemeData dark;
	static ThemeData *theme;
	static Mode mode;

	static Colorf mix(const ColorLayers& color, float mix);

public:
	static void initilize(Mode mode);

	static Mode getMode() { return Theme::mode; }
	static void setMode(Mode mode)
	{
		Theme::mode = mode;
		theme = mode == LIGHT? &light: &dark;
	}

	static Colorf getMain(float mix);
	static Colorf getHeader(float mix);
	static Colorf getButton(float mix);
	static Colorf getHover(float mix);
	static Colorf getClick(float mix);
	static Colorf getBorder(float mix);
};
