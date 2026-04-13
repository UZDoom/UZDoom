/*
** system_theme.h
**
** Gets system theme from operating system
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

enum InterfaceTheme
{
	Default = 0,

	Light = 1,
	Dark = 2,

	HighContrast = 8,
	HighContrastLight = HighContrast | Light,
	HighContrastDark = HighContrast | Dark,

	ColorScheme = HighContrast - 1,
};

InterfaceTheme GetSystemTheme();
