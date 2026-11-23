/*
** theme.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <QColor>
#include <QString>
#include <string>
#include <vector>

enum class LauncherThemeCategory
{
	System = 0,
	Preset = 1,
	Custom = 2
};

struct LauncherThemePalette
{
	QColor windowBg, surfaceBg, surfaceAltBg, elevatedBg;
	QColor textPrimary, textSecondary, textDisabled, link;
	QColor accent, accentText, buttonBg, buttonText, border, focusBorder;
	QColor favoriteStar, updateBannerBg, updateBannerText;
	QColor logErrorFg, logErrorBg, logWarningFg, logWarningBg, logHeaderFg;
	QColor genericThumbBg, genericThumbFg;
};

struct LauncherThemeDefinition
{
	std::string id;
	QString name;
	const char* l10nKey = nullptr;
	LauncherThemeCategory category = LauncherThemeCategory::Preset;
	bool isDark = true;
	bool isHighContrast = false;
	bool isAuto = false;
	LauncherThemePalette palette;
};

std::string normalizeThemeId(const std::string& rawId);
std::vector<LauncherThemeDefinition> getAllLauncherThemes();
QString getLauncherThemeDisplayName(const LauncherThemeDefinition& def);
bool isAutoLauncherTheme(const std::string& themeId);
bool isDarkLauncherTheme(const std::string& themeId);
void applyLauncherTheme(const std::string& themeId);
const LauncherThemeDefinition& getActiveLauncherTheme();
