/*
** theme.cpp
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

#include "theme.h"
#include "const.h"

#include "i_specialpaths.h"
#include "system_theme.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPalette>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QStyleHints>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

namespace
{
	LauncherThemeDefinition g_activeTheme;
	bool g_hasActiveTheme = false;

	struct ColorFieldMeta
	{
		const char* key;
		QColor LauncherThemePalette::*member;
		bool allowAlpha;
	};

	const ColorFieldMeta kColorFields[] = {
		{"windowBg", &LauncherThemePalette::windowBg, false},
		{"surfaceBg", &LauncherThemePalette::surfaceBg, false},
		{"surfaceAltBg", &LauncherThemePalette::surfaceAltBg, false},
		{"elevatedBg", &LauncherThemePalette::elevatedBg, false},
		{"textPrimary", &LauncherThemePalette::textPrimary, false},
		{"textSecondary", &LauncherThemePalette::textSecondary, false},
		{"textDisabled", &LauncherThemePalette::textDisabled, false},
		{"link", &LauncherThemePalette::link, false},
		{"accent", &LauncherThemePalette::accent, false},
		{"accentText", &LauncherThemePalette::accentText, false},
		{"buttonBg", &LauncherThemePalette::buttonBg, false},
		{"buttonText", &LauncherThemePalette::buttonText, false},
		{"border", &LauncherThemePalette::border, false},
		{"focusBorder", &LauncherThemePalette::focusBorder, false},
		{"favoriteStar", &LauncherThemePalette::favoriteStar, false},
		{"updateBannerBg", &LauncherThemePalette::updateBannerBg, false},
		{"updateBannerText", &LauncherThemePalette::updateBannerText, false},
		{"logErrorFg", &LauncherThemePalette::logErrorFg, false},
		{"logErrorBg", &LauncherThemePalette::logErrorBg, true},
		{"logWarningFg", &LauncherThemePalette::logWarningFg, false},
		{"logWarningBg", &LauncherThemePalette::logWarningBg, true},
		{"logHeaderFg", &LauncherThemePalette::logHeaderFg, false},
		{"genericThumbBg", &LauncherThemePalette::genericThumbBg, false},
		{"genericThumbFg", &LauncherThemePalette::genericThumbFg, false},
	};

	QColor mixColor(const QColor& a, const QColor& b, double t)
	{
		const double k = std::clamp(t, 0.0, 1.0);
		return QColor(
			qRound(a.red() * (1.0 - k) + b.red() * k),
			qRound(a.green() * (1.0 - k) + b.green() * k),
			qRound(a.blue() * (1.0 - k) + b.blue() * k),
			qRound(a.alpha() * (1.0 - k) + b.alpha() * k));
	}

	double relativeLuminance(const QColor& c)
	{
		auto lin = [](double ch)
		{
			const double s = ch / 255.0;
			return (s <= 0.03928) ? (s / 12.92) : std::pow((s + 0.055) / 1.055, 2.4);
		};
		return 0.2126 * lin(c.red()) + 0.7152 * lin(c.green()) + 0.0722 * lin(c.blue());
	}

	QColor pickReadableText(const QColor& bg)
	{
		return (relativeLuminance(bg) > 0.36) ? QColor(18, 18, 20) : QColor(255, 255, 255);
	}

	QColor shiftL(const QColor& c, int delta)
	{
		int h = 0, s = 0, l = 0, a = 255;
		c.getHsl(&h, &s, &l, &a);
		QColor out;
		out.setHsl(h, s, std::clamp(l + delta, 0, 255), a);
		return out;
	}

	QString css(const QColor& c)
	{
		if (!c.isValid())
			return QStringLiteral("#000000");
		return (c.alpha() < 255)
			? QString("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha())
			: c.name(QColor::HexRgb);
	}

	bool parseHex(const QString& raw, QColor& out, bool allowAlpha)
	{
		QString s = raw.trimmed();
		if (s.isEmpty())
			return false;
		if (!s.startsWith(QLatin1Char('#')))
			s.prepend(QLatin1Char('#'));
		if (!allowAlpha && s.size() == 9)
			s = QStringLiteral("#") + s.mid(3);
		if (s.size() != 7 && (!allowAlpha || s.size() != 9) && s.size() != 4)
			return false;
		const QColor parsed(s);
		if (!parsed.isValid())
			return false;
		out = allowAlpha ? parsed : QColor(parsed.red(), parsed.green(), parsed.blue(), 255);
		return true;
	}

	QString themesDir()
	{
		if (!LauncherConst::THEMES_DIR.empty())
			return QDir::cleanPath(QString::fromStdString(LauncherConst::THEMES_DIR));
		if (!LauncherConst::ROOT_DIR.empty())
			return QDir(QString::fromStdString(LauncherConst::ROOT_DIR)).filePath(QStringLiteral("themes"));
		const FString appData = M_GetAppDataPath(false);
		return appData.IsNotEmpty()
			? QDir(QDir::cleanPath(QString::fromUtf8(appData.GetChars()))).filePath(QStringLiteral("launcher/themes"))
			: QString();
	}

	bool isSystemDark()
	{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
		if (const auto* hints = QGuiApplication::styleHints())
		{
			const Qt::ColorScheme scheme = hints->colorScheme();
			if (scheme != Qt::ColorScheme::Unknown)
				return scheme == Qt::ColorScheme::Dark;
		}
#endif
		// Cache GetSystemTheme() so Linux doesn't repeatedly spawn gdbus on UI refreshes.
		static const InterfaceTheme sysTheme = GetSystemTheme();
		return (sysTheme & ColorScheme) != Light;
	}

	LauncherThemePalette makeDarkBase()
	{
		return {
			{36, 39, 44}, {26, 29, 33}, {33, 36, 41}, {32, 35, 40},
			{235, 238, 243}, {199, 205, 214}, {125, 130, 138}, {90, 170, 255},
			{47, 111, 222}, {255, 255, 255}, {48, 52, 60}, {235, 238, 243}, {68, 73, 82}, {52, 118, 209},
			{245, 197, 66}, {11, 83, 148}, {255, 255, 255},
			{255, 128, 128}, {220, 50, 50, 42}, {245, 199, 84}, {230, 175, 40, 32}, {123, 180, 247},
			{34, 38, 44}, {114, 124, 140}
		};
	}

	LauncherThemePalette makeLightBase()
	{
		return {
			{239, 239, 239}, {255, 255, 255}, {245, 247, 250}, {230, 233, 239},
			{17, 19, 23}, {48, 52, 59}, {135, 142, 152}, {30, 95, 190},
			{42, 110, 200}, {255, 255, 255}, {232, 235, 240}, {17, 19, 23}, {184, 191, 204}, {42, 110, 200},
			{214, 148, 0}, {11, 83, 148}, {255, 255, 255},
			{180, 24, 24}, {220, 45, 45, 24}, {140, 90, 0}, {230, 165, 0, 26}, {27, 90, 166},
			{244, 246, 250}, {124, 133, 148}
		};
	}

	LauncherThemePalette makeHighContrastBase()
	{
		return {
			{0, 0, 0}, {10, 10, 10}, {24, 24, 24}, {0, 0, 0},
			{255, 255, 255}, {255, 255, 255}, {140, 140, 140}, {0, 255, 255},
			{255, 255, 0}, {0, 0, 0}, {20, 20, 20}, {255, 255, 255}, {255, 255, 255}, {255, 255, 0},
			{255, 255, 0}, {0, 0, 0}, {255, 255, 0},
			{255, 102, 102}, {0, 0, 0, 0}, {255, 255, 0}, {0, 0, 0, 0}, {0, 255, 255},
			{10, 10, 10}, {255, 255, 0}
		};
	}

	LauncherThemePalette deriveHarmonized(const QColor& accentIn, const QColor& bgTintIn, bool dark, int tintPct)
	{
		const QColor accent = accentIn.isValid() ? QColor(accentIn.red(), accentIn.green(), accentIn.blue()) : QColor(52, 118, 209);
		const QColor tint = bgTintIn.isValid() ? QColor(bgTintIn.red(), bgTintIn.green(), bgTintIn.blue()) : accent;
		const double strength = std::clamp(tintPct, 0, 100) / 100.0;
		int h = 0, s = 0, l = 0;
		tint.getHsl(&h, &s, &l);
		h = std::max(0, h);

		auto surf = [&](int lum, double satScale) -> QColor
		{
			QColor c;
			c.setHsl(h, std::clamp(qRound(s * strength * satScale), 0, dark ? 115 : 85), std::clamp(lum, 0, 255));
			return c;
		};

		LauncherThemePalette p = dark ? makeDarkBase() : makeLightBase();
		if (dark)
		{
			p.windowBg = surf(34, 0.42);
			p.surfaceBg = surf(24, 0.40);
			p.surfaceAltBg = surf(31, 0.40);
			p.elevatedBg = surf(28, 0.44);
			p.textPrimary = mixColor({236, 239, 244}, tint, 0.07 * strength);
			p.textSecondary = mixColor({192, 198, 208}, tint, 0.14 * strength);
			p.textDisabled = mixColor({122, 127, 136}, tint, 0.14 * strength);
			p.link = shiftL(accent, 42);
			if (relativeLuminance(p.link) < 0.25)
				p.link = mixColor(accent, {255, 255, 255}, 0.42);
			p.buttonBg = surf(48, 0.44);
			p.border = mixColor(surf(72, 0.44), accent, 0.18);
			p.focusBorder = shiftL(accent, 14);
			p.updateBannerBg = mixColor(p.surfaceBg, accent, 0.56);
			p.genericThumbBg = surf(28, 0.42);
			p.genericThumbFg = mixColor(surf(116, 0.40), accent, 0.42);
		}
		else
		{
			p.windowBg = surf(237, 0.36);
			p.surfaceBg = surf(250, 0.25);
			p.surfaceAltBg = surf(242, 0.32);
			p.elevatedBg = surf(228, 0.38);
			p.textPrimary = surf(24, 0.30);
			p.textSecondary = surf(76, 0.26);
			p.textDisabled = surf(142, 0.20);
			p.link = shiftL(accent, -16);
			if (relativeLuminance(p.link) > 0.35)
				p.link = mixColor(accent, {12, 12, 16}, 0.35);
			p.buttonBg = surf(227, 0.36);
			p.border = mixColor(surf(182, 0.34), accent, 0.16);
			p.focusBorder = accent;
			p.updateBannerBg = mixColor(accent, {20, 20, 24}, 0.22);
			p.genericThumbBg = surf(232, 0.34);
			p.genericThumbFg = mixColor(surf(126, 0.32), accent, 0.35);
		}

		p.accent = accent;
		p.accentText = pickReadableText(accent);
		p.buttonText = p.textPrimary;
		p.updateBannerText = pickReadableText(p.updateBannerBg);
		p.logHeaderFg = p.link;
		return p;
	}

	const std::vector<LauncherThemeDefinition>& builtInThemes()
	{
		static const std::vector<LauncherThemeDefinition> themes = []()
		{
			std::vector<LauncherThemeDefinition> list = {
				{"auto", QStringLiteral("Auto"), "OPTVAL_AUTO", LauncherThemeCategory::System, true, false, true, makeDarkBase()},
				{"dark", QStringLiteral("Dark"), "OPTVAL_DARK", LauncherThemeCategory::System, true, false, false, makeDarkBase()},
				{"light", QStringLiteral("Light"), "OPTVAL_LIGHT", LauncherThemeCategory::System, false, false, false, makeLightBase()},
				{"highcontrast", QStringLiteral("High Contrast"), "PICKER_THEME_HIGHCONTRAST", LauncherThemeCategory::System, true, true, false, makeHighContrastBase()},
			};

			struct Preset { const char* id; const char* name; QColor accent; QColor tint; bool dark; int strength; QColor link; QColor star; };
			const Preset presets[] = {
				{"crimson",     "Crimson Red",     {201, 42, 42},  {158, 34, 42},  true, 70, {255, 130, 112}, {255, 178, 52}},
				{"forest",      "Forest Green",    {61, 153, 76},  {46, 128, 62},  true, 68, {126, 224, 129}, {217, 230, 76}},
				{"steel_amber", "Steel Amber",     {217, 120, 30}, {68, 92, 130},  true, 60, {108, 184, 255}, {255, 190, 59}},
				{"violet",      "Midnight Violet", {176, 42, 82},  {104, 68, 152}, true, 65, {194, 140, 255}, {240, 184, 73}},
				{"gold",        "Antique Gold",    {194, 142, 39}, {136, 100, 48}, true, 62, {112, 184, 255}, {255, 210, 77}},
				{"obsidian",    "Obsidian Blue",   {56, 132, 176}, {56, 84, 122},  true, 60, {112, 214, 255}, {140, 224, 255}},
				{"copper",      "Warm Copper",     {196, 106, 43}, {120, 110, 68}, true, 58, {104, 199, 186}, {242, 183, 56}},
			};

			for (const auto& s : presets)
			{
				LauncherThemeDefinition def{s.id, QString::fromUtf8(s.name), nullptr, LauncherThemeCategory::Preset, s.dark, false, false,
					deriveHarmonized(s.accent, s.tint, s.dark, s.strength)};
				def.palette.link = s.link;
				def.palette.logHeaderFg = s.link;
				def.palette.favoriteStar = s.star;
				list.push_back(std::move(def));
			}
			return list;
		}();
		return themes;
	}

	std::vector<LauncherThemeDefinition> loadCustomThemes()
	{
		std::vector<LauncherThemeDefinition> out;
		const QDir dir(themesDir());
		if (!dir.exists())
			return out;

		for (const QFileInfo& fi : dir.entryInfoList({QStringLiteral("*.json")}, QDir::Files | QDir::Readable))
		{
			QFile f(fi.absoluteFilePath());
			if (!f.open(QIODevice::ReadOnly))
				continue;
			const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
			const QJsonObject root = doc.object();
			const QJsonObject colors = root.value(QStringLiteral("colors")).toObject();
			if (!doc.isObject() || colors.isEmpty())
				continue;

			LauncherThemeDefinition def;
			def.name = root.value(QStringLiteral("name")).toString().trimmed();
			if (def.name.isEmpty())
				def.name = fi.completeBaseName();
			QString slug = root.value(QStringLiteral("id")).toString().trimmed().toLower();
			if (slug.isEmpty())
				slug = def.name.toLower();
			slug.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]+")), QStringLiteral("_"));
			def.id = slug.toStdString();
			if (def.id.empty())
				continue;

			def.category = LauncherThemeCategory::Custom;
			def.isDark = root.value(QStringLiteral("isDark")).toBool(true);
			def.isHighContrast = root.value(QStringLiteral("isHighContrast")).toBool(false);

			QColor hintAccent, hintBg;
			if (def.isHighContrast)
				def.palette = makeHighContrastBase();
			else if (parseHex(colors.value(QStringLiteral("accent")).toString(), hintAccent, false)
				&& parseHex(colors.value(QStringLiteral("windowBg")).toString(), hintBg, false))
				def.palette = deriveHarmonized(hintAccent, hintBg, def.isDark, 62);
			else
				def.palette = def.isDark ? makeDarkBase() : makeLightBase();

			for (const auto& fMeta : kColorFields)
			{
				QColor parsed;
				if (parseHex(colors.value(QLatin1String(fMeta.key)).toString(), parsed, fMeta.allowAlpha))
					def.palette.*(fMeta.member) = parsed;
			}
			out.push_back(std::move(def));
		}

		std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.name.compare(b.name, Qt::CaseInsensitive) < 0; });
		return out;
	}

	LauncherThemeDefinition resolveTheme(const std::string& themeId)
	{
		const std::string norm = normalizeThemeId(themeId);
		const auto& builtIns = builtInThemes();
		auto resolveAuto = [&]()
		{
			const std::string target = isSystemDark() ? "dark" : "light";
			for (const auto& t : builtIns)
			{
				if (t.id == target)
				{
					LauncherThemeDefinition copy = t;
					copy.isAuto = true;
					return copy;
				}
			}
			return builtIns.front();
		};
		if (norm == "auto")
			return resolveAuto();
		for (const auto& t : builtIns)
			if (t.id == norm) return t;
		for (const auto& t : loadCustomThemes())
			if (normalizeThemeId(t.id) == norm) return t;
		return resolveAuto();
	}

	QPalette buildQPalette(const LauncherThemeDefinition& def)
	{
		const LauncherThemePalette& c = def.palette;
		const bool hc = def.isHighContrast;

		QPalette pal(c.buttonBg, c.windowBg);
		pal.setColor(QPalette::Window, c.windowBg);
		pal.setColor(QPalette::WindowText, c.textPrimary);
		pal.setColor(QPalette::Base, c.surfaceBg);
		pal.setColor(QPalette::AlternateBase, c.surfaceAltBg);
		pal.setColor(QPalette::ToolTipBase, hc ? QColor(0, 0, 0) : (def.isDark ? mixColor(c.surfaceBg, c.windowBg, 0.2) : QColor(250, 251, 253)));
		pal.setColor(QPalette::ToolTipText, hc ? c.accent : c.textPrimary);
		pal.setColor(QPalette::Text, c.textPrimary);
		pal.setColor(QPalette::PlaceholderText, hc ? QColor(170, 170, 170) : mixColor(c.textDisabled, c.textPrimary, 0.15));
		pal.setColor(QPalette::Button, c.buttonBg);
		pal.setColor(QPalette::ButtonText, c.buttonText);
		pal.setColor(QPalette::BrightText, hc ? c.accent : (def.isDark ? QColor(255, 110, 110) : QColor(175, 25, 25)));
		pal.setColor(QPalette::Link, c.link);
		pal.setColor(QPalette::Highlight, c.accent);
		pal.setColor(QPalette::HighlightedText, c.accentText);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
		pal.setColor(QPalette::Accent, c.accent);
#endif
		pal.setColor(QPalette::Mid, hc ? QColor(160, 160, 160) : c.border);

		if (!def.isDark)
		{
			pal.setColor(QPalette::Light, QColor(255, 255, 255));
			pal.setColor(QPalette::Midlight, mixColor(c.buttonBg, c.windowBg, 0.5));
			pal.setColor(QPalette::Dark, shiftL(c.border, -32));
			pal.setColor(QPalette::Shadow, shiftL(c.border, -72));
		}

		pal.setColor(QPalette::Disabled, QPalette::WindowText, c.textDisabled);
		pal.setColor(QPalette::Disabled, QPalette::Text, c.textDisabled);
		pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, hc ? QColor(115, 115, 115) : mixColor(c.textDisabled, c.surfaceBg, 0.30));
		pal.setColor(QPalette::Disabled, QPalette::ButtonText, c.textDisabled);
		return pal;
	}

	QString buildStyleSheet(const LauncherThemeDefinition& def)
	{
		const LauncherThemePalette& c = def.palette;
		const bool hc = def.isHighContrast;
		const bool dark = def.isDark;

		const QColor mbHoverBg = hc ? c.accent : (dark ? shiftL(c.buttonBg, 4) : shiftL(c.elevatedBg, -18));
		const QColor mbHoverFg = hc ? QColor(0, 0, 0) : (dark ? QColor(255, 255, 255) : c.textPrimary);
		const QColor hdrBg = hc ? QColor(20, 20, 20) : shiftL(c.windowBg, 8);
		const QColor hdrBdr = hc ? c.border : mixColor(c.border, c.windowBg, 0.15);
		const QColor tabBg = hc ? QColor(20, 20, 20) : (dark ? shiftL(c.windowBg, 9) : shiftL(c.windowBg, -10));
		const QColor tabHover = shiftL(tabBg, 8);
		const QColor tabSelBg = hc ? QColor(0, 0, 0) : (dark ? shiftL(c.windowBg, 22) : c.windowBg);
		const QColor tabSelFg = hc ? c.accent : (dark ? QColor(255, 255, 255) : c.textPrimary);
		const QColor accHover = hc ? QColor(255, 255, 255) : shiftL(c.accent, 16);
		const QColor priDisBg = hc ? QColor(32, 32, 32) : mixColor(c.buttonBg, c.border, 0.35);
		const QColor priDisFg = hc ? c.textDisabled : (dark ? mixColor(c.textDisabled, c.textPrimary, 0.35) : QColor(240, 242, 245));
		const QColor banBdr = hc ? c.accent : shiftL(c.updateBannerBg, dark ? 20 : -20);
		const QColor banHov = hc ? c.accent : shiftL(c.updateBannerBg, 16);
		const QColor banPrs = hc ? QColor(255, 255, 255) : shiftL(c.updateBannerBg, -14);

		QString qss;
		if (hc || dark)
		{
			qss += QString("QToolTip{color:%1;background-color:%2;border:1px solid %3;padding:4px;}")
				.arg(css(hc ? c.accent : c.textPrimary), css(hc ? QColor(0, 0, 0) : mixColor(c.surfaceBg, c.windowBg, 0.25)), css(hc ? c.accent : shiftL(c.border, 8)));
		}

		qss += QString(
			"QMenuBar{background-color:%1;color:%2;border-bottom:1px solid %3;padding:2px 4px;}"
			"QMenuBar::item{padding:5px 12px;border-radius:%4px;background:transparent;}"
			"QMenuBar::item:selected{background-color:%5;color:%6;}"
			"QGroupBox{border:1px solid %7;border-radius:%8px;margin-top:8px;padding:10px 6px 6px 6px;font-weight:600;}"
			"QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 5px;}"
			"QTableView,QListWidget,QTextEdit,QPlainTextEdit,QScrollArea,#launcherCoverLabel{border:1px solid %7;border-radius:3px;}"
			"QTableView:focus,QListWidget:focus,QTextEdit:focus,QPlainTextEdit:focus,QScrollArea:focus{border:1px solid %9;}"
			"QTableView::item{padding:2px 8px;}"
		).arg(css(c.elevatedBg), css(c.textPrimary), css(hc ? c.border : mixColor(c.border, c.elevatedBg, 0.35))).arg(hc ? 0 : 4)
		 .arg(css(mbHoverBg), css(mbHoverFg), css(c.border)).arg(hc ? 4 : 5).arg(css(c.focusBorder));

		if (hc || dark)
		{
			qss += QString("QHeaderView::section{background-color:%1;color:%2;padding:5px 10px;font-weight:600;border:none;border-right:1px solid %3;border-bottom:1px solid %3;}")
				.arg(css(hdrBg), css(c.textPrimary), css(hdrBdr));
		}
		else
		{
			qss += QStringLiteral("QHeaderView::section{padding:5px 10px;font-weight:600;}");
		}

		qss += QString(
			"QTabWidget::pane{border:1px solid %1;border-radius:4px;top:-1px;}"
			"QTabBar::tab{background:%2;color:%3;padding:6px 14px;border:1px solid %1;border-bottom:%4;border-top-left-radius:4px;border-top-right-radius:4px;margin-right:2px;}"
			"QTabBar::tab:hover:!selected{background:%5;border-color:%6;}"
			"QTabBar::tab:selected{background:%7;color:%8;border:1px solid %9;border-bottom:1px solid %7;font-weight:%10;}"
		).arg(css(c.border), css(tabBg), css(c.textSecondary), dark ? QStringLiteral("none") : (QStringLiteral("1px solid ") + css(c.border)))
		 .arg(css(hc ? tabBg : tabHover), css(hc ? c.accent : c.border), css(tabSelBg), css(tabSelFg), css(hc ? c.accent : c.border), hc ? QStringLiteral("bold") : QStringLiteral("600"));

		if (hc)
		{
			qss += QString(
				"QPushButton{border:1px solid %1;border-radius:4px;padding:4px 10px;}"
				"QPushButton:hover{border-color:%2;}"
				"QPushButton:disabled{color:%3;border-color:#707070;}"
			).arg(css(c.border), css(c.accent), css(c.textDisabled));
		}

		qss += QString(
			"QPushButton[launcherRole=\"primary\"]{background-color:%1;color:%2;%3border-radius:4px;font-weight:bold;}"
			"QPushButton[launcherRole=\"primary\"]:hover{background-color:%4;color:%5;%6}"
			"QPushButton[launcherRole=\"primary\"]:disabled{background-color:%7;color:%8;%9}"
		).arg(css(c.accent), css(c.accentText), hc ? (QStringLiteral("border:2px solid ") + css(c.border) + QLatin1Char(';')) : QString())
		 .arg(css(accHover), css(pickReadableText(accHover)), hc ? (QStringLiteral("border-color:") + css(c.accent) + QLatin1Char(';')) : QString())
		 .arg(css(priDisBg), css(priDisFg), hc ? (QStringLiteral("border-color:") + css(c.textDisabled) + QLatin1Char(';')) : QString());

		qss += QString(
			"#launcherUpdateBanner{background-color:%1;border:1px solid %2;border-radius:4px;}"
			"#launcherUpdateBanner QToolButton{background-color:transparent;border:none;border-right:1px solid %2;border-top-left-radius:3px;border-bottom-left-radius:3px;}"
			"#launcherUpdateBanner QToolButton:hover{background-color:#d94336;}#launcherUpdateBanner QToolButton:pressed{background-color:#a82d24;}"
			"#launcherUpdateBanner QPushButton{background-color:transparent;color:%3;border:none;font-weight:bold;text-align:center;padding:0 12px;border-top-right-radius:3px;border-bottom-right-radius:3px;}"
			"#launcherUpdateBanner QPushButton:hover{background-color:%4;color:%5;}#launcherUpdateBanner QPushButton:pressed{background-color:%6;}"
		).arg(css(c.updateBannerBg), css(banBdr), css(c.updateBannerText), css(banHov), css(hc ? QColor(0, 0, 0) : c.updateBannerText), css(banPrs));

		return qss;
	}
}

std::string normalizeThemeId(const std::string& rawId)
{
	const QString lower = QString::fromStdString(rawId).trimmed().toLower();
	if (lower.isEmpty() || lower == QStringLiteral("auto") || lower == QStringLiteral("system"))
		return "auto";
	if (lower == QStringLiteral("high_contrast") || lower == QStringLiteral("contrast"))
		return "highcontrast";
	return lower.toStdString();
}

std::vector<LauncherThemeDefinition> getAllLauncherThemes()
{
	std::vector<LauncherThemeDefinition> all = builtInThemes();
	auto custom = loadCustomThemes();
	all.insert(all.end(), std::make_move_iterator(custom.begin()), std::make_move_iterator(custom.end()));
	return all;
}

QString getLauncherThemeDisplayName(const LauncherThemeDefinition& def)
{
	return (def.l10nKey && *def.l10nKey) ? LStr(def.l10nKey, def.name.toUtf8().constData()) : def.name;
}

bool isAutoLauncherTheme(const std::string& themeId) { return normalizeThemeId(themeId) == "auto"; }
bool isDarkLauncherTheme(const std::string& themeId) { return resolveTheme(themeId).isDark; }

void applyLauncherTheme(const std::string& themeId)
{
	const LauncherThemeDefinition def = resolveTheme(themeId);
	g_activeTheme = def;
	g_hasActiveTheme = true;

	QApplication* app = qobject_cast<QApplication*>(QCoreApplication::instance());
	if (!app)
		return;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
	if (auto* hints = app->styleHints())
	{
		if (def.isAuto) hints->unsetColorScheme();
		else hints->setColorScheme(def.isDark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
	}
#endif
	app->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
	app->setPalette(buildQPalette(def));
	app->setStyleSheet(buildStyleSheet(def));
}

const LauncherThemeDefinition& getActiveLauncherTheme()
{
	if (!g_hasActiveTheme)
	{
		g_activeTheme = resolveTheme(LauncherConst::DEFAULT_THEME);
		g_hasActiveTheme = true;
	}
	return g_activeTheme;
}
