/*
** const.h
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

#include <QByteArray>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class QApplication;

inline QString qStr(const std::string& s)
{
	return QString::fromUtf8(s.c_str(), static_cast<int>(s.size()));
}

inline std::string stdStr(const QString& s)
{
	const QByteArray utf8 = s.toUtf8();
	return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

namespace LauncherConst
{
	inline std::string ROOT_DIR;
	inline std::string EXEC_DIR;
	inline std::string PROFILE_DIR;
	inline std::string CONFIG_FILE;

	inline constexpr const char* DEFAULT_LANG = "auto";
	inline constexpr const char* DEFAULT_THEME = "auto";
	inline constexpr const char* DEFAULT_BANNER = "default";

}

struct WadInfo
{
	std::string title;
	std::string author;
	std::string releaseDate;
	bool isIWAD = false;
};

struct FlagChoice
{
	QString label;
	uint32_t bitVal = 0;
};

struct FlagInfo
{
	QString label;
	QString tooltip;
	int setIdx = 0;
	uint32_t bitVal = 0;
	uint32_t bitMask = 0;
	bool invert = false;
	std::vector<FlagChoice> choices;
};

struct CompatModeInfo
{
	int id = 0;
	QString label;
	uint32_t compatflags = 0;
	uint32_t compatflags2 = 0;
};

struct EngineComplevelInfo
{
	QString value;
	QString label;
};

struct LauncherLanguage
{
	QString code;
	QString name;
};

enum class LauncherTheme
{
	Auto = 0,
	Dark = 1,
	Light = 2,
	HighContrast = 3
};

QApplication* ensureLauncherQtApplication();
QString LStr(const char* key, const char* fallback = nullptr);

inline QString ensureTrailingColon(QString s)
{
	const QString trimmed = s.trimmed();
	if (trimmed.isEmpty() || trimmed.endsWith(QLatin1Char(':')) || trimmed.endsWith(QChar(0xFF1A)))
		return trimmed;
	return trimmed + QLatin1Char(':');
}

inline QString stripTrailingColon(QString s)
{
	QString trimmed = s.trimmed();
	while (trimmed.endsWith(QLatin1Char(':')) || trimmed.endsWith(QChar(0xFF1A)))
		trimmed.chop(1);
	return trimmed.trimmed();
}

const std::vector<LauncherLanguage>& getLauncherLanguages();
LauncherTheme parseThemeName(const std::string& theme);
std::string themeToName(LauncherTheme theme);
bool isDarkThemeActive(LauncherTheme theme);
void applyLauncherTheme(const std::string& themeName);

std::unordered_map<std::string, QByteArray> readBaseWadEntries(const std::vector<const char*>& entryNames);
QByteArray readBaseWadEntry(const char* entryName);

QIcon makeLauncherIcon(const QString& iconKind, const QColor& color, int size = 18);
QPixmap loadUZDoomLogoPixmap(int size = 52);
QPixmap loadUZDoomGenericThumbnailPixmap(int width, int height, qreal dpr = 0.0);

std::vector<FlagInfo> getDMFlagsList();
std::vector<FlagInfo> getCompatFlagsList();
std::vector<CompatModeInfo> getCompatModes();
bool findCompatModeById(int id, CompatModeInfo& out);
int findMatchingCompatModeId(uint32_t compatflags, uint32_t compatflags2);
std::vector<EngineComplevelInfo> getEngineComplevels();

const std::unordered_map<std::string, WadInfo>& getWadDatabase();

struct DiscoveredIWad
{
	std::string name;
	std::string path;
};
inline std::vector<DiscoveredIWad> g_discoveredEngineIWads;
