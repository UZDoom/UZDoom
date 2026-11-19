/*
** const.cpp
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

#include "const.h"

#include "doomdef.h"
#include "filesystem.h"
#include "findfile.h"
#include "gameconfigfile.h"
#include "gstrings.h"
#include "i_specialpaths.h"
#include "m_argv.h"
#include "resourcefile.h"
#include "sc_man.h"
#include "version.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QGuiApplication>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmapCache>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <algorithm>
#include <cmath>
#include <memory>

namespace
{
	std::string loadSavedLauncherTheme()
	{
		QString configPath;
		if (!LauncherConst::CONFIG_FILE.empty())
		{
			configPath = QString::fromStdString(LauncherConst::CONFIG_FILE);
		}
		else
		{
			const FString appData = M_GetAppDataPath(false);
			if (appData.IsNotEmpty())
			{
				const QString appDataRoot = QDir::cleanPath(QString::fromUtf8(appData.GetChars()));
				configPath = QDir(appDataRoot).filePath("launcher/config.json");
			}
		}

		if (configPath.isEmpty())
			return LauncherConst::DEFAULT_THEME;

		QFile f(configPath);
		if (!f.open(QIODevice::ReadOnly))
			return LauncherConst::DEFAULT_THEME;

		const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
		if (!doc.isObject())
			return LauncherConst::DEFAULT_THEME;

		const QString theme = doc.object().value("theme").toString().trimmed();
		return theme.isEmpty() ? LauncherConst::DEFAULT_THEME : theme.toStdString();
	}
}

QApplication* ensureLauncherQtApplication()
{
	if (QApplication* existing = qobject_cast<QApplication*>(QCoreApplication::instance()))
		return existing;

	static int s_argc = 0;
	static std::vector<std::string> s_argStrings;
	static std::vector<char*> s_argv;
	static std::unique_ptr<QApplication> s_ownedApp;

	if (!s_ownedApp)
	{
		s_argStrings.clear();
		s_argv.clear();
		if (Args && Args->NumArgs() > 0)
		{
			const int count = Args->NumArgs();
			s_argStrings.reserve(static_cast<size_t>(count));
			for (int i = 0; i < count; ++i)
				s_argStrings.emplace_back(Args->GetArg(i));
		}
		else
		{
			s_argStrings.emplace_back(GAMENAMELOWERCASE);
		}

		s_argv.reserve(s_argStrings.size() + 1);
		for (std::string& arg : s_argStrings)
			s_argv.push_back(arg.data());
		s_argv.push_back(nullptr);
		s_argc = static_cast<int>(s_argStrings.size());

		s_ownedApp = std::make_unique<QApplication>(s_argc, s_argv.data());
		QCoreApplication::setApplicationName(GAMENAME);
		QCoreApplication::setOrganizationName("UZDoom");
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
		QGuiApplication::setDesktopFileName(APPID);
#endif
		QFont appFont = s_ownedApp->font();
		appFont.setStyleStrategy(QFont::PreferAntialias);
		s_ownedApp->setFont(appFont);
		s_ownedApp->setWindowIcon(QIcon(loadUZDoomLogoPixmap(64)));
		QGuiApplication::setQuitOnLastWindowClosed(false);
		applyLauncherTheme(loadSavedLauncherTheme());
	}

	return s_ownedApp.get();
}

QString LStr(const char* key, const char* fallback)
{
	if (key && *key)
	{
		if (const char* s = GStrings.CheckString(key); s && *s)
			return QString::fromUtf8(s);
	}
	if (fallback && *fallback)
		return QString::fromUtf8(fallback);
	return key ? QString::fromUtf8(key) : QString();
}

std::unordered_map<std::string, QByteArray> readBaseWadEntries(const std::vector<const char*>& entryNames)
{
	std::unordered_map<std::string, QByteArray> entries;
	if (entryNames.empty())
		return entries;

	const char* wad = BaseFileSearch(BASEWAD, nullptr, true, GameConfig);
	if (!wad)
		return entries;

	std::unique_ptr<FResourceFile> resf(FResourceFile::OpenResourceFile(wad));
	if (!resf)
		return entries;

	for (const char* name : entryNames)
	{
		if (!name || !*name)
			continue;
		const int lump = resf->FindEntry(name);
		if (lump < 0)
			continue;
		auto data = resf->Read(lump);
		if (data.size() > 0)
			entries.emplace(name, QByteArray(data.string(), static_cast<int>(data.size())));
	}

	return entries;
}

QByteArray readBaseWadEntry(const char* entryName)
{
	auto entries = readBaseWadEntries({entryName});
	const auto it = entries.find(entryName ? entryName : "");
	return (it != entries.end()) ? it->second : QByteArray();
}

const std::vector<LauncherLanguage>& getLauncherLanguages()
{
	static const std::vector<LauncherLanguage> cached = []()
	{
		std::vector<LauncherLanguage> result;

		const QByteArray raw = readBaseWadEntry("menudef.txt");
		if (!raw.isEmpty())
		{
			FScanner sc;
			sc.OpenMem("menudef.txt", raw.constData(), raw.size());
			while (sc.GetString())
			{
				if (sc.Compare("OptionString"))
				{
					sc.MustGetString();
					if (sc.Compare("LanguageOptions"))
					{
						sc.MustGetStringName("{");
						while (!sc.CheckString("}"))
						{
							sc.MustGetString();
							QString iso = QString::fromUtf8(sc.String);
							sc.MustGetStringName(",");
							sc.MustGetString();
							QString label = QString::fromUtf8(sc.String);
							result.push_back({iso, label});
						}
						break;
					}
				}
			}
		}

		if (!result.empty())
			return result;

		return std::vector<LauncherLanguage>{
			{"auto", "Auto"},
			{"enu", "English (US)"},
			{"eng", "English (UK)"},
			{"cs", "Cesky"},
			{"da", "Dansk"},
			{"de", "Deutsch"},
			{"es", "Espanol"},
			{"esm", "Espanol (Mexico)"},
			{"eo", "Esperanto"},
			{"fi", "Suomi"},
			{"fr", "Francais"},
			{"hu", "Magyar"},
			{"it", "Italiano"},
			{"jp", "Japanese"},
			{"ko", "Korean"},
			{"nl", "Nederlands"},
			{"no", "Norsk"},
			{"pl", "Polski"},
			{"ptg", "Portugues (Portugal)"},
			{"pt", "Portugues (Brasil)"},
			{"ro", "Romana"},
			{"ru", "Russkiy"},
			{"sr", "Srpski"},
			{"sv", "Svenska"},
			{"tr", "Turkce"},
			{"uk", "Ukrainska"},
			{"chs", "Chinese (Simplified)"},
			{"cht", "Chinese (Traditional)"}
		};
	}();

	return cached;
}

LauncherTheme parseThemeName(const std::string& theme)
{
	QString lower = QString::fromStdString(theme).trimmed().toLower();
	if (lower == "dark")
		return LauncherTheme::Dark;
	if (lower == "light")
		return LauncherTheme::Light;
	if (lower == "highcontrast" || lower == "high_contrast" || lower == "contrast")
		return LauncherTheme::HighContrast;
	return LauncherTheme::Auto;
}

std::string themeToName(LauncherTheme theme)
{
	switch (theme)
	{
	case LauncherTheme::Dark:
		return "dark";
	case LauncherTheme::Light:
		return "light";
	case LauncherTheme::HighContrast:
		return "highcontrast";
	case LauncherTheme::Auto:
	default:
		return "auto";
	}
}

bool isDarkThemeActive(LauncherTheme theme)
{
	if (theme == LauncherTheme::Dark || theme == LauncherTheme::HighContrast)
		return true;
	if (theme == LauncherTheme::Light)
		return false;

	if (const auto* hints = QGuiApplication::styleHints())
		return hints->colorScheme() == Qt::ColorScheme::Dark;
	return true;
}

void applyLauncherTheme(const std::string& themeName)
{
	QApplication* app = qobject_cast<QApplication*>(QCoreApplication::instance());
	if (!app)
		return;

	app->setStyle(QStyleFactory::create("Fusion"));
	const LauncherTheme theme = parseThemeName(themeName);

	QPalette pal;
	if (theme == LauncherTheme::HighContrast)
	{
		pal.setColor(QPalette::Window, QColor(0, 0, 0));
		pal.setColor(QPalette::WindowText, QColor(255, 255, 255));
		pal.setColor(QPalette::Base, QColor(10, 10, 10));
		pal.setColor(QPalette::AlternateBase, QColor(24, 24, 24));
		pal.setColor(QPalette::ToolTipBase, QColor(0, 0, 0));
		pal.setColor(QPalette::ToolTipText, QColor(255, 255, 0));
		pal.setColor(QPalette::Text, QColor(255, 255, 255));
		pal.setColor(QPalette::PlaceholderText, QColor(170, 170, 170));
		pal.setColor(QPalette::Button, QColor(20, 20, 20));
		pal.setColor(QPalette::ButtonText, QColor(255, 255, 255));
		pal.setColor(QPalette::BrightText, QColor(255, 255, 0));
		pal.setColor(QPalette::Link, QColor(0, 255, 255));
		pal.setColor(QPalette::Highlight, QColor(255, 255, 0));
		pal.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
		pal.setColor(QPalette::Mid, QColor(160, 160, 160));
		pal.setColor(QPalette::Disabled, QPalette::Text, QColor(140, 140, 140));
		pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, QColor(115, 115, 115));
		pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(140, 140, 140));
	}
	else if (isDarkThemeActive(theme))
	{
		pal.setColor(QPalette::Window, QColor(36, 39, 44));
		pal.setColor(QPalette::WindowText, QColor(235, 238, 243));
		pal.setColor(QPalette::Base, QColor(26, 29, 33));
		pal.setColor(QPalette::AlternateBase, QColor(33, 36, 41));
		pal.setColor(QPalette::ToolTipBase, QColor(24, 26, 30));
		pal.setColor(QPalette::ToolTipText, QColor(235, 238, 243));
		pal.setColor(QPalette::Text, QColor(235, 238, 243));
		pal.setColor(QPalette::PlaceholderText, QColor(140, 148, 160));
		pal.setColor(QPalette::Button, QColor(48, 52, 60));
		pal.setColor(QPalette::ButtonText, QColor(235, 238, 243));
		pal.setColor(QPalette::BrightText, QColor(255, 110, 110));
		pal.setColor(QPalette::Link, QColor(90, 170, 255));
		pal.setColor(QPalette::Highlight, QColor(52, 118, 209));
		pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
		pal.setColor(QPalette::Mid, QColor(68, 73, 82));
		pal.setColor(QPalette::Disabled, QPalette::Text, QColor(125, 130, 138));
		pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, QColor(95, 100, 108));
		pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(125, 130, 138));
	}
	else
	{
		pal = app->style()->standardPalette();
		pal.setColor(QPalette::Highlight, QColor(42, 110, 200));
		pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
		pal.setColor(QPalette::BrightText, QColor(175, 25, 25));
	}
	app->setPalette(pal);

	if (theme == LauncherTheme::HighContrast)
	{
		app->setStyleSheet(
			"QToolTip { color: #ffff00; background-color: #000000; border: 1px solid #ffff00; }"
			"QMenuBar { background-color: #000000; color: #ffffff; border-bottom: 1px solid #ffffff; padding: 2px 4px; }"
			"QMenuBar::item { padding: 5px 12px; background: transparent; }"
			"QMenuBar::item:selected { background-color: #ffff00; color: #000000; }"
			"QGroupBox { border: 1px solid #ffffff; border-radius: 4px; margin-top: 8px; padding: 10px 6px 6px 6px; }"
			"QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }"
			"QTableView, QListWidget, QTextEdit, QPlainTextEdit, QScrollArea, #launcherCoverLabel { border: 1px solid #ffffff; border-radius: 3px; }"
			"QTableView:focus, QListWidget:focus, QTextEdit:focus, QPlainTextEdit:focus, QScrollArea:focus { border: 1px solid #ffff00; }"
			"QHeaderView::section { background-color: #141414; color: #ffffff; padding: 5px 10px; font-weight: 600; border: none; border-right: 1px solid #ffffff; border-bottom: 1px solid #ffffff; }"
			"QTableView::item { padding: 2px 8px; }"
			"QTabWidget::pane { border: 1px solid #ffffff; border-radius: 4px; top: -1px; }"
			"QTabBar::tab { background: #141414; color: #ffffff; padding: 6px 14px; border: 1px solid #ffffff; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; margin-right: 2px; }"
			"QTabBar::tab:hover:!selected { border-color: #ffff00; }"
			"QTabBar::tab:selected { background: #000000; color: #ffff00; border: 1px solid #ffff00; border-bottom: 1px solid #000000; font-weight: bold; }"
			"QPushButton { border: 1px solid #ffffff; border-radius: 4px; padding: 4px 10px; }"
			"QPushButton:hover { border-color: #ffff00; }"
			"QPushButton:disabled { color: #8c8c8c; border-color: #707070; }"
			"QPushButton[launcherRole=\"primary\"] { background-color: #ffff00; color: #000000; border: 2px solid #ffffff; font-weight: bold; }"
			"QPushButton[launcherRole=\"primary\"]:hover { background-color: #ffffff; color: #000000; border-color: #ffff00; }"
			"QPushButton[launcherRole=\"primary\"]:disabled { background-color: #202020; color: #8c8c8c; border-color: #8c8c8c; }"
			"QPushButton[launcherRole=\"danger\"] { background-color: #000000; color: #ffff00; border: 2px solid #ffff00; font-weight: bold; padding: 4px 10px; }"
			"QPushButton[launcherRole=\"danger\"]:hover { background-color: #ffff00; color: #000000; }"
		);
	}
	else if (isDarkThemeActive(theme))
	{
		app->setStyleSheet(
			"QToolTip { color: #ebeff5; background-color: #1c1f24; border: 1px solid #4c5360; padding: 4px; }"
			"QMenuBar { background-color: #202328; color: #ebeff5; border-bottom: 1px solid #3c414a; padding: 2px 4px; }"
			"QMenuBar::item { padding: 5px 12px; border-radius: 4px; background: transparent; }"
			"QMenuBar::item:selected { background-color: #343942; color: #ffffff; }"
			"QGroupBox { border: 1px solid #444952; border-radius: 5px; margin-top: 8px; padding: 10px 6px 6px 6px; font-weight: 600; }"
			"QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
			"QTableView, QListWidget, QTextEdit, QPlainTextEdit, QScrollArea, #launcherCoverLabel { border: 1px solid #444952; border-radius: 3px; }"
			"QTableView:focus, QListWidget:focus, QTextEdit:focus, QPlainTextEdit:focus, QScrollArea:focus { border: 1px solid #3476d1; }"
			"QHeaderView::section { background-color: #2c3037; color: #ebeff5; padding: 5px 10px; font-weight: 600; border: none; border-right: 1px solid #3f444e; border-bottom: 1px solid #3f444e; }"
			"QTableView::item { padding: 2px 8px; }"
			"QTabWidget::pane { border: 1px solid #444952; border-radius: 4px; }"
			"QTabBar::tab { background: #2d3138; color: #c7cdd6; padding: 6px 14px; border: 1px solid #444952; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; margin-right: 2px; }"
			"QTabBar::tab:selected { background: #3b4049; color: #ffffff; font-weight: 600; }"
			"QPushButton[launcherRole=\"primary\"] { background-color: #2f6fde; color: #ffffff; border-radius: 4px; font-weight: bold; }"
			"QPushButton[launcherRole=\"primary\"]:hover { background-color: #3d7ee8; }"
			"QPushButton[launcherRole=\"primary\"]:disabled { background-color: #4f5765; color: #a6adb8; }"
			"QPushButton[launcherRole=\"danger\"] { background-color: #a93226; color: #ffffff; border: 1px solid #782016; border-radius: 3px; padding: 3px 10px; }"
			"QPushButton[launcherRole=\"danger\"]:hover { background-color: #c0392b; border-color: #96281b; }"
			"QPushButton[launcherRole=\"danger\"]:pressed { background-color: #87251c; }"
		);
	}
	else
	{
		app->setStyleSheet(
			"QMenuBar { background-color: #e6e9ef; border-bottom: 1px solid #c6ccd6; padding: 2px 4px; }"
			"QMenuBar::item { padding: 5px 12px; border-radius: 4px; background: transparent; }"
			"QMenuBar::item:selected { background-color: #d2d8e2; }"
			"QGroupBox { border: 1px solid #b8bfcc; border-radius: 5px; margin-top: 8px; padding: 10px 6px 6px 6px; font-weight: 600; }"
			"QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
			"QTableView, QListWidget, QTextEdit, QPlainTextEdit, QScrollArea, #launcherCoverLabel { border: 1px solid #b8bfcc; border-radius: 3px; }"
			"QTableView:focus, QListWidget:focus, QTextEdit:focus, QPlainTextEdit:focus, QScrollArea:focus { border: 1px solid #2a6ec8; }"
			"QHeaderView::section { padding: 5px 10px; font-weight: 600; }"
			"QTableView::item { padding: 2px 8px; }"
			"QTabWidget::pane { border: 1px solid #b8bfcc; border-radius: 4px; top: -1px; }"
			"QTabBar::tab { background: #e2e5eb; color: #30343b; padding: 6px 14px; border: 1px solid #b8bfcc; border-bottom: 1px solid #b8bfcc; border-top-left-radius: 4px; border-top-right-radius: 4px; margin-right: 2px; }"
			"QTabBar::tab:hover:!selected { background: #eaedf2; }"
			"QTabBar::tab:selected { background: #efefef; color: #111317; border-bottom: 1px solid #efefef; font-weight: 600; }"
			"QPushButton[launcherRole=\"primary\"] { background-color: #2f6fde; color: #ffffff; border-radius: 4px; font-weight: bold; }"
			"QPushButton[launcherRole=\"primary\"]:hover { background-color: #3d7ee8; }"
			"QPushButton[launcherRole=\"primary\"]:disabled { background-color: #b4becd; color: #f0f2f5; }"
			"QPushButton[launcherRole=\"danger\"] { background-color: #c0392b; color: #ffffff; border: 1px solid #96281b; border-radius: 3px; padding: 3px 10px; }"
			"QPushButton[launcherRole=\"danger\"]:hover { background-color: #d64536; border-color: #782016; }"
			"QPushButton[launcherRole=\"danger\"]:pressed { background-color: #96281b; }"
		);
	}
}

QIcon makeLauncherIcon(const QString& iconKind, const QColor& color, int size)
{
	const qreal dpr = (qApp && qApp->devicePixelRatio() > 0.0) ? qApp->devicePixelRatio() : 1.0;
	const int physSize = std::max(1, static_cast<int>(std::ceil(size * dpr)));
	const QString cacheKey = QString("uz_icon_%1_%2_%3_%4")
		.arg(iconKind)
		.arg(color.rgba(), 8, 16, QLatin1Char('0'))
		.arg(size)
		.arg(physSize);

	QPixmap pm;
	if (QPixmapCache::find(cacheKey, &pm))
		return QIcon(pm);

	pm = QPixmap(physSize, physSize);
	const qreal actualDpr = static_cast<qreal>(physSize) / static_cast<qreal>(size);
	pm.setDevicePixelRatio(actualDpr);
	pm.fill(Qt::transparent);

	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	QPen pen(color, std::max(1.5, size / 10.0), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	p.setPen(pen);
	p.setBrush(Qt::NoBrush);

	const qreal m = size * 0.18;
	const qreal w = size - 2.0 * m;
	const qreal h = size - 2.0 * m;
	const qreal cx = size * 0.5;
	const qreal cy = size * 0.5;

	if (iconKind == "up")
	{
		QPolygonF tri;
		tri << QPointF(cx, m + h * 0.15) << QPointF(m + w * 0.88, m + h * 0.82) << QPointF(m + w * 0.12, m + h * 0.82);
		p.setBrush(color);
		p.drawPolygon(tri);
	}
	else if (iconKind == "down")
	{
		QPolygonF tri;
		tri << QPointF(m + w * 0.12, m + h * 0.18) << QPointF(m + w * 0.88, m + h * 0.18) << QPointF(cx, m + h * 0.85);
		p.setBrush(color);
		p.drawPolygon(tri);
	}
	else if (iconKind == "delete")
	{
		p.drawLine(QPointF(m + w * 0.15, m + h * 0.15), QPointF(m + w * 0.85, m + h * 0.85));
		p.drawLine(QPointF(m + w * 0.85, m + h * 0.15), QPointF(m + w * 0.15, m + h * 0.85));
	}
	else if (iconKind == "kick")
	{
		p.drawEllipse(QPointF(cx, m + h * 0.25), w * 0.2, h * 0.2);
		p.drawLine(QPointF(cx - w * 0.25, m + h * 0.85), QPointF(cx + w * 0.15, m + h * 0.5));
		p.drawLine(QPointF(cx + w * 0.2, cy), QPointF(m + w, cy));
		p.drawLine(QPointF(m + w * 0.8, cy - h * 0.15), QPointF(m + w, cy));
		p.drawLine(QPointF(m + w * 0.8, cy + h * 0.15), QPointF(m + w, cy));
	}
	else if (iconKind == "ban")
	{
		p.drawEllipse(QRectF(m, m, w, h));
		p.drawLine(QPointF(m + w * 0.18, m + h * 0.18), QPointF(m + w * 0.82, m + h * 0.82));
	}
	p.end();

	QPixmapCache::insert(cacheKey, pm);
	return QIcon(pm);
}

namespace
{
	QPainterPath buildUZDoomEmblemPath()
	{
		QPainterPath emblem;
		emblem.setFillRule(Qt::OddEvenFill);
		emblem.moveTo(1500.0, 2751.1);
		emblem.lineTo(557.3, 2207.0);
		emblem.lineTo(557.3, 248.9);
		emblem.lineTo(1272.7, 661.9);
		emblem.lineTo(1272.7, 945.8);
		emblem.lineTo(1015.4, 796.9);
		emblem.lineTo(1015.4, 1811.3);
		emblem.lineTo(653.9, 2075.7);
		emblem.lineTo(1050.6, 1914.7);
		emblem.lineTo(1499.9, 2174.0);
		emblem.lineTo(1949.2, 1914.7);
		emblem.lineTo(2345.9, 2075.7);
		emblem.lineTo(1984.4, 1811.3);
		emblem.lineTo(1984.4, 796.9);
		emblem.lineTo(1727.1, 945.8);
		emblem.lineTo(1727.1, 661.9);
		emblem.lineTo(2442.5, 248.9);
		emblem.lineTo(2442.5, 2207.0);
		emblem.closeSubpath();

		emblem.moveTo(1727.3, 1068.6);
		emblem.lineTo(1914.1, 960.9);
		emblem.lineTo(1914.1, 1811.4);
		emblem.lineTo(1500.0, 2051.3);
		emblem.lineTo(1086.0, 1811.4);
		emblem.lineTo(1086.0, 960.9);
		emblem.lineTo(1272.8, 1068.6);
		emblem.lineTo(1272.8, 1750.4);
		emblem.lineTo(1499.9, 1881.4);
		emblem.lineTo(1727.2, 1750.4);
		emblem.closeSubpath();
		return emblem;
	}
}

QPixmap loadUZDoomLogoPixmap(int size)
{
	const qreal dpr = (qApp && qApp->devicePixelRatio() > 0.0) ? qApp->devicePixelRatio() : 1.0;
	const int physSize = std::max(1, static_cast<int>(std::ceil(size * dpr)));
	const QString cacheKey = QString("uz_logo_%1_%2").arg(size).arg(physSize);

	QPixmap cachedPm;
	if (QPixmapCache::find(cacheKey, &cachedPm))
		return cachedPm;

	const qreal actualDpr = static_cast<qreal>(physSize) / static_cast<qreal>(size);
	QPixmap pm(physSize, physSize);
	pm.setDevicePixelRatio(actualDpr);
	pm.fill(Qt::transparent);

	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(0x23, 0x78, 0x87));
	const qreal radius = size * 0.18;
	p.drawRoundedRect(QRectF(0.0, 0.0, size, size), radius, radius);

	const qreal scale = static_cast<qreal>(size) / 3000.0;
	p.scale(scale, scale);

	p.setBrush(Qt::white);
	p.drawPath(buildUZDoomEmblemPath());
	p.end();

	QPixmapCache::insert(cacheKey, pm);
	return pm;
}

QPixmap loadUZDoomGenericThumbnailPixmap(int width, int height, qreal dpr)
{
	const int safeW = std::max(1, width);
	const int safeH = std::max(1, height);
	const qreal resolvedDpr = (dpr > 0.0)
		? dpr
		: ((qApp && qApp->devicePixelRatio() > 0.0) ? qApp->devicePixelRatio() : 1.0);
	const int physW = std::max(1, static_cast<int>(std::ceil(safeW * resolvedDpr)));
	const int physH = std::max(1, static_cast<int>(std::ceil(safeH * resolvedDpr)));
	const QString cacheKey = QString("uz_generic_thumb_%1_%2_%3_%4").arg(safeW).arg(safeH).arg(physW).arg(physH);

	QPixmap cachedPm;
	if (QPixmapCache::find(cacheKey, &cachedPm))
		return cachedPm;

	const qreal actualDpr = static_cast<qreal>(physH) / static_cast<qreal>(safeH);
	QPixmap pm(physW, physH);
	pm.setDevicePixelRatio(actualDpr);
	pm.fill(Qt::white);

	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(127, 127, 127));

	const qreal logicalW = static_cast<qreal>(physW) / actualDpr;
	const qreal logicalH = static_cast<qreal>(physH) / actualDpr;
	const qreal scale = std::min(logicalW, logicalH) / 3000.0;
	p.translate(logicalW * 0.5, logicalH * 0.5);
	p.scale(scale, scale);
	p.translate(-1500.0, -1500.0);
	p.drawPath(buildUZDoomEmblemPath());
	p.end();

	QPixmapCache::insert(cacheKey, pm);
	return pm;
}

const std::unordered_map<std::string, WadInfo>& getWadDatabase()
{
	static const std::unordered_map<std::string, WadInfo> wadDatabase = {
		{"90facab21eede7981be10790e3f82da2", {"Doom (Shareware 1.0)", "id Software", "10-12-1993", true}},
		{"cea4989df52b65f4d481b706234a3dca", {"Doom (Shareware 1.1)", "id Software", "15-12-1993", true}},
		{"52cbc8882f445573ce421fa5453513c1", {"Doom (Shareware 1.1)", "id Software", "16-12-1993", true}},
		{"2a380f28e813fb0989cae5e4762ebb4c", {"Doom (Shareware 1.2)", "id Software", "04-02-1994", true}},
		{"30aa5beb9e5ebfbbe1e1765561c08f38", {"Doom (Shareware 1.2)", "id Software", "17-02-1994", true}},
		{"17aebd6b5f2ed8ce07aa526a32af8d99", {"Doom (Shareware 1.25)", "id Software", "21-04-1994", true}},
		{"a21ae40c388cb6f2c3cc1b95589ee693", {"Doom (Shareware 1.4)", "id Software", "28-06-1994", true}},
		{"e280233d533dcc28c1acd6ccdc7742d4", {"Doom (Shareware 1.5)", "id Software", "08-07-1994", true}},
		{"762fd6d4b960d4b759730f01387a50a1", {"Doom (Shareware 1.6)", "id Software", "03-08-1994", true}},
		{"c428ea394dc52835f2580d5bfd50d76f", {"Doom (Shareware 1.666)", "id Software", "30-08-1994", true}},
		{"5f4eb849b1af12887dec04a2a12e5e62", {"Doom (Shareware 1.8)", "id Software", "10-12-1994", true}},
		{"f0cefca49926d00903cf57551d901abe", {"Doom (Shareware 1.9)", "id Software", "01-02-1995", true}},
		{"740901119ba2953e3c7f3764eca6e128", {"Doom (Registered 0.2)", "id Software", "04-02-1993", true}},
		{"dae9b1eea1a8e090fdfa5707187f4a43", {"Doom (Registered 0.3)", "id Software", "28-02-1993", true}},
		{"b6afa12a8b22e2726a8ff5bd249223de", {"Doom (Registered 0.4)", "id Software", "03-04-1993", true}},
		{"9c877480b8ef33b7074f1f0c07ed6487", {"Doom (Registered 0.5)", "id Software", "23-05-1993", true}},
		{"049e32f18d9c9529630366cfc72726ea", {"Doom (Registered Beta)", "id Software", "04-10-1993", true}},
		{"981b03e6d1dc033301aa3095acc437ce", {"Doom (Registered 1.1)", "id Software", "16-12-1993", true}},
		{"792fd1fea023d61210857089a7c1e351", {"Doom (Registered 1.2)", "id Software", "17-02-1994", true}},
		{"54978d12de87f162b9bcc011676cb3c0", {"Doom (Registered 1.666)", "id Software", "01-09-1994", true}},
		{"11e1cd216801ea2657723abc86ecb01f", {"Doom (Registered 1.8)", "id Software", "20-01-1995", true}},
		{"1cd63c5ddff1bf8ce844237f580e9cf3", {"Doom (Registered 1.9)", "id Software", "01-02-1995", true}},
		{"c4fe9fd920207691a9f493668e0a2083", {"The Ultimate Doom", "id Software", "25-05-1995", true}},
		{"fb35c4a5a9fd49ec29ab6e900572c524", {"Doom (BFG Edition)", "id Software", "16-10-2012", true}},
		{"8517c4e8f0eef90b82852667d345eb86", {"The Ultimate Doom (Unity/Bethesda)", "id Software", "09-01-2020", true}},
		{"d9153ced9fd5b898b36cc5844e35b520", {"Doom II (1.666 German)", "id Software", "29-08-1994", true}},
		{"30e3c2d0350b67bfbf47271970b74b2f", {"Doom II (1.666)", "id Software", "29-08-1994", true}},
		{"ea74a47a791fdef2e9f2ea8b8a9da13b", {"Doom II (1.7)", "id Software", "21-09-1994", true}},
		{"d7a07e5d3f4625074312bc299d7ed33f", {"Doom II (1.7a)", "id Software", "18-10-1994", true}},
		{"3cb02349b3df649c86290907eed64e7b", {"Doom II (1.8 French)", "id Software", "01-12-1994", true}},
		{"c236745bb01d89bbb866c8fed81b6f8c", {"Doom II (1.8)", "id Software", "20-01-1995", true}},
		{"25e1459ca71d321525f84628f45ca8cd", {"Doom II (1.9)", "id Software", "01-02-1995", true}},
		{"b96683d113c4f4e9a916e1c7d1d71ffd", {"Doom II (PC-98)", "id Software", "01-02-1995", true}},
		{"c3bea40570c23e511a7ed3ebcd9865f7", {"Doom II (BFG Edition)", "id Software", "16-10-2012", true}},
		{"8ab6d0527a29efdc1ef200e5687b5cae", {"Doom II (Unity/Bethesda)", "id Software", "09-01-2020", true}},
		{"4e158d9953c79ccf97bd0663244cc6b6", {"Final Doom: TNT Evilution (1.9)", "TeamTNT", "10-06-1996", true}},
		{"1d39e405bf6ee3df69a8d2646c8d5c49", {"Final Doom: TNT Evilution (Anthology)", "TeamTNT", "14-11-1996", true}},
		{"f5528f6fd55cf9629141d79eda169630", {"Final Doom: TNT Evilution (Unity/Bethesda)", "TeamTNT", "03-09-2020", true}},
		{"75c8cf89566741fa9d22447604053bd7", {"Final Doom: Plutonia Experiment", "Casali Brothers", "10-06-1996", true}},
		{"3493be7e1e2588bc9c8b31eab2587a04", {"Final Doom: Plutonia Experiment (Anthology)", "Casali Brothers", "21-11-1996", true}},
		{"ae76c20366ff685d3bb9fab11b148b84", {"Final Doom: Plutonia Experiment (Unity/Bethesda)", "Casali Brothers", "03-09-2020", true}},
		{"ae779722390ec32fa37b0d361f7d82f8", {"Heretic (Shareware 1.2)", "Raven Software", "28-06-1995", true}},
		{"023b52175d2f260c3bdc5528df5d0a8c", {"Heretic (Shareware 1.0)", "Raven Software", "24-12-1994", true}},
		{"fc7eab659f6ee522bb57acc1a946912f", {"Heretic (Shareware Beta)", "Raven Software", "23-12-1994", true}},
		{"66d686b1ed6d35ff103f15dbd30e0341", {"Heretic (1.3)", "Raven Software", "22-03-1996", true}},
		{"1e4cb4ef075ad344dd63971637307e04", {"Heretic (1.2)", "Raven Software", "28-06-1995", true}},
		{"3117e399cdb4298eaa3941625f4b2923", {"Heretic (1.0)", "Raven Software", "27-12-1994", true}},
		{"abb033caf81e26f12a2103e1fa25453f", {"Hexen: Beyond Heretic (1.1)", "Raven Software", "14-03-1996", true}},
		{"b2543a03521365261d0a0f74d5dd90f0", {"Hexen: Beyond Heretic (1.0)", "Raven Software", "13-10-1995", true}},
		{"c88a2bb3d783e2ad7b599a8e301e099e", {"Hexen: Beyond Heretic (Beta)", "Raven Software", "27-09-1995", true}},
		{"876a5a44c7b68f04b3bb9bc7a5bd69d6", {"Hexen: Beyond Heretic (Demo)", "Raven Software", "18-10-1995", true}},
		{"9178a32a496ff5befebfe6c47dac106c", {"Hexen: Beyond Heretic (Demo Beta)", "Raven Software", "02-10-1995", true}},
		{"78d5898e99e220e4de64edaa0e479593", {"Hexen: Deathkings of the Dark Citadel (1.1)", "Raven Software", "09-05-1996", false}},
		{"1077432e2690d390c256ac908b5f4efa", {"Hexen: Deathkings of the Dark Citadel (1.0)", "Raven Software", "22-03-1996", false}},
		{"bb545b9c4eca0ff92c14d466b3294023", {"Strife (Teaser 1.1)", "Rogue Entertainment", "14-03-1996", true}},
		{"de2c8dcad7cca206292294bdab524292", {"Strife (Teaser 1.0)", "Rogue Entertainment", "22-02-1996", true}},
		{"2fed2031a5b03892106e0f117f17901f", {"Strife (1.2 - 1.31)", "Rogue Entertainment", "23-05-1996", true}},
		{"8f2d3a6a289f5d2f2f9c1eec02b47299", {"Strife (1.1)", "Rogue Entertainment", "18-04-1996", true}},
		{"082234d6a3f7086424856478b5aa9e95", {"Strife (Voices)", "Rogue Entertainment", "18-04-1996", false}},
		{"bce163d06521f9d15f9686786e64df13", {"Chex Quest 3 (1.4)", "Charles Jacobi", "24-06-2009", true}},
		{"148367e53ff7f4f814e54b5ac9ff0ab3", {"Chex Quest 3 (1.3)", "Charles Jacobi", "12-06-2009", true}},
		{"26a8998ecdaa983f8e6c363b4b95bf55", {"Chex Quest 3 (1.2)", "Charles Jacobi", "02-05-2009", true}},
		{"f85944f55fff094f2ffbd3ecef3fa255", {"Chex Quest 3 (1.1)", "Charles Jacobi", "22-04-2009", true}},
		{"cb001c34e424687191f299cc1dff4d68", {"Chex Quest 3 (1.0)", "Charles Jacobi", "12-11-2008", true}},
		{"59c985995db55cd2623c1893550d82b3", {"Chex Quest 3 (1.0 unofficial PWAD)", "Charles Jacobi", "24-06-2009", true}},
		{"1914b280b0a4b517214523bc2270e758", {"Action Doom 2: Urban Brawl (1.0)", "Stephen Browning et al.", "??-??-????", true}},
		{"c106a4e0a96f299954b073d5f97240be", {"Action Doom 2: Urban Brawl (1.1)", "Stephen Browning et al.", "26-12-2013", true}},
		{"402ca45bb90520bfef0dec6baac5889e", {"Hacx (1.0 verified)", "Banjo Software", "08-10-1997", true}},
		{"1511a7032ebc834a3884cf390d7f186e", {"Hacx (1.0 unverified)", "Banjo Software", "09-10-1997", true}},
		{"b7fd2f43f3382cf012dc6b097a3cb182", {"Hacx (1.1)", "Banjo Software", "16-09-1997", true}},
		{"65ed74d522bdf6649c2831b13b9e02b4", {"Hacx (1.2)", "Banjo Software", "09-10-2010", true}},
		{"793f07ebadb3d7353ee5b6b6429d9afa", {"Hacx (2.0)", "Banjo Software et al.", "09-10-2010", true}},
		{"48ebb49b52f6a3020d174dbcc1b9aeaf", {"Harmony (1.1)", "Thomas van der Velden", "17-02-2012", true}},
		{"fe2cce6713ddcf6c6d6f0e8154b0cb38", {"Harmony (1.0)", "Thomas van der Velden", "10-12-2009", true}},
		{"f4578097c658ad3c813cd5901ec125e2", {"The Adventures of Square (2.1)", "BigBrik Games", "22-06-2019", true}},
		{"a185498bdf721b4c01dc87fa81d1580b", {"Delaweare", "Space Is Green", "30-06-2014", true}},
		{"9176043468e10eaa471ae556e8e55745", {"Rise of the Wool Ball (1.3)", "MSPaintR0cks", "21-07-2017", true}},
		{"fb0226e3fed7a3c1e7ea4cd4da905950", {"Rise of the Wool Ball (1.2)", "MSPaintR0cks", "14-06-2017", true}},
		{"b93be13d05148dd01614bc205a03648e", {"Freedoom: Phase 1 (0.13)", "Freedoom Project", "30-01-2024", true}},
		{"cd666466759b5e5f63af93c5f0ffd0a1", {"Freedoom: Phase 2 (0.13)", "Freedoom Project", "30-01-2024", true}},
		{"908dfd77a14cc490c4cea94b62d13449", {"FreeDM (0.13)", "Freedoom Project", "30-01-2024", true}},
	};
	return wadDatabase;
}

std::vector<CompatModeInfo> getCompatModes()
{
	return {
		{0, LStr("OPTVAL_DEFAULT", "Default"), 0, 0},
		{1, LStr("OPTVAL_DOOM", "Doom"),
			COMPATF_SHORTTEX | COMPATF_STAIRINDEX | COMPATF_USEBLOCKING | COMPATF_NODOORLIGHT | COMPATF_SPRITESORT |
			COMPATF_TRACE | COMPATF_MISSILECLIP | COMPATF_SOUNDTARGET | COMPATF_DEHHEALTH | COMPATF_CROSSDROPOFF |
			COMPATF_LIGHT | COMPATF_MASKEDMIDTEX,
			COMPATF2_FLOORMOVE | COMPATF2_EXPLODE1 | COMPATF2_RESERVEDLINEFLAG},
		{2, LStr("OPTVAL_DOOMSTRICT", "Doom (strict)"),
			COMPATF_SHORTTEX | COMPATF_STAIRINDEX | COMPATF_USEBLOCKING | COMPATF_NODOORLIGHT | COMPATF_SPRITESORT |
			COMPATF_TRACE | COMPATF_MISSILECLIP | COMPATF_SOUNDTARGET | COMPATF_NO_PASSMOBJ | COMPATF_LIMITPAIN |
			COMPATF_DEHHEALTH | COMPATF_INVISIBILITY | COMPATF_CROSSDROPOFF | COMPATF_VILEGHOSTS | COMPATF_HITSCAN |
			COMPATF_WALLRUN | COMPATF_NOTOSSDROPS | COMPATF_LIGHT | COMPATF_MASKEDMIDTEX,
			COMPATF2_BADANGLES | COMPATF2_FLOORMOVE | COMPATF2_POINTONLINE | COMPATF2_EXPLODE2 | COMPATF2_VOODOO_ZOMBIES |
			COMPATF2_EMULATEMIKOPORTALS | COMPATF2_RESERVEDLINEFLAG},
		{3, LStr("OPTVAL_BOOM", "Boom"),
			COMPATF_TRACE | COMPATF_SOUNDTARGET | COMPATF_BOOMSCROLL | COMPATF_MISSILECLIP | COMPATF_MASKEDMIDTEX,
			COMPATF2_EXPLODE1 | COMPATF2_RESERVEDLINEFLAG | COMPATF2_TRANSFERSECRET},
		{6, LStr("OPTVAL_BOOMSTRICT", "Boom (strict)"),
			COMPATF_TRACE | COMPATF_SOUNDTARGET | COMPATF_BOOMSCROLL | COMPATF_MISSILECLIP | COMPATF_NO_PASSMOBJ |
			COMPATF_INVISIBILITY | COMPATF_HITSCAN | COMPATF_WALLRUN | COMPATF_NOTOSSDROPS | COMPATF_MASKEDMIDTEX,
			COMPATF2_POINTONLINE | COMPATF2_EXPLODE2 | COMPATF2_EMULATEMIKOPORTALS | COMPATF2_RESERVEDLINEFLAG |
			COMPATF2_TRANSFERSECRET},
		{5, LStr("OPTVAL_MBF", "MBF"),
			COMPATF_TRACE | COMPATF_SOUNDTARGET | COMPATF_BOOMSCROLL | COMPATF_MISSILECLIP | COMPATF_MUSHROOM |
			COMPATF_MBFMONSTERMOVE | COMPATF_NOBLOCKFRIENDS | COMPATF_MASKEDMIDTEX,
			COMPATF2_EXPLODE1 | COMPATF2_AVOID_HAZARDS | COMPATF2_STAYONLIFT | COMPATF2_RESERVEDLINEFLAG},
		{7, LStr("OPTVAL_MBFSTRICT", "MBF (strict)"),
			COMPATF_NOBLOCKFRIENDS | COMPATF_MBFMONSTERMOVE | COMPATF_INVISIBILITY |
			COMPATF_NOTOSSDROPS | COMPATF_MUSHROOM | COMPATF_NO_PASSMOBJ | COMPATF_BOOMSCROLL | COMPATF_WALLRUN |
			COMPATF_TRACE | COMPATF_HITSCAN | COMPATF_MISSILECLIP | COMPATF_MASKEDMIDTEX | COMPATF_SOUNDTARGET,
			COMPATF2_POINTONLINE | COMPATF2_EXPLODE1 | COMPATF2_EXPLODE2 | COMPATF2_AVOID_HAZARDS | COMPATF2_STAYONLIFT |
			COMPATF2_RESERVEDLINEFLAG | COMPATF2_TRANSFERSECRET},
		{8, LStr("OPTVAL_MBF21", "MBF21"),
			COMPATF_TRACE | COMPATF_SOUNDTARGET | COMPATF_BOOMSCROLL | COMPATF_MISSILECLIP | COMPATF_CROSSDROPOFF |
			COMPATF_MUSHROOM | COMPATF_MBFMONSTERMOVE | COMPATF_NOBLOCKFRIENDS | COMPATF_MASKEDMIDTEX,
			COMPATF2_EXPLODE1 | COMPATF2_AVOID_HAZARDS | COMPATF2_STAYONLIFT | COMPATF2_RESERVEDLINEFLAG |
			COMPATF2_TRANSFERSECRET},
		{9, LStr("OPTVAL_MBF21STRICT", "MBF21 (strict)"),
			COMPATF_NOBLOCKFRIENDS | COMPATF_MBFMONSTERMOVE | COMPATF_INVISIBILITY |
			COMPATF_NOTOSSDROPS | COMPATF_MUSHROOM | COMPATF_NO_PASSMOBJ | COMPATF_BOOMSCROLL | COMPATF_WALLRUN |
			COMPATF_TRACE | COMPATF_HITSCAN | COMPATF_MISSILECLIP | COMPATF_CROSSDROPOFF | COMPATF_MASKEDMIDTEX | COMPATF_SOUNDTARGET,
			COMPATF2_POINTONLINE | COMPATF2_EXPLODE1 | COMPATF2_EXPLODE2 | COMPATF2_AVOID_HAZARDS | COMPATF2_STAYONLIFT |
			COMPATF2_RESERVEDLINEFLAG | COMPATF2_EMULATEMIKOPORTALS | COMPATF2_TRANSFERSECRET},
		{4, LStr("OPTVAL_ZDOOM2063", "ZDoom 2.0.63"),
			COMPATF_SOUNDTARGET | COMPATF_LIGHT,
			COMPATF2_MULTIEXIT | COMPATF2_TELEPORT | COMPATF2_PUSHWINDOW | COMPATF2_CHECKSWITCHRANGE |
			COMPATF2_RESERVEDLINEFLAG | COMPATF2_TRANSFERSECRET},
	};
}

bool findCompatModeById(int id, CompatModeInfo& out)
{
	for (const CompatModeInfo& cm : getCompatModes())
	{
		if (cm.id == id)
		{
			out = cm;
			return true;
		}
	}
	return false;
}

int findMatchingCompatModeId(uint32_t compatflags, uint32_t compatflags2)
{
	for (const CompatModeInfo& cm : getCompatModes())
	{
		if (cm.compatflags == compatflags && cm.compatflags2 == compatflags2)
			return cm.id;
	}
	return -1;
}

std::vector<EngineComplevelInfo> getEngineComplevels()
{
	return {
		{"", LStr("OPTVAL_DEFAULT", "Default")},
		{"vanilla", "Vanilla (2 / 3 / 4)"},
		{"boom", "Boom (9)"},
		{"mbf", "MBF (11)"},
		{"mbf21", "MBF21 (21)"},
	};
}

std::vector<FlagInfo> getDMFlagsList()
{
	return {
		{LStr("GMPLYMNU_ALLOWHEALTH", "Allow health"), "Do not spawn health items (DM) when unchecked.", 0, DF_NO_HEALTH, 0, true},
		{LStr("GMPLYMNU_ALLOWPOWERUPS", "Allow powerups"), "Do not spawn powerups (DM) when unchecked.", 0, DF_NO_ITEMS, 0, true},
		{LStr("GMPLYMNU_WEAPONSSTAY", "Weapons stay"), "Leave weapons around after pickup (DM).", 0, DF_WEAPONS_STAY, 0, false},
		{LStr("GMPLYMNU_FALLINGDAMAGE", "Falling damage"), "Control whether falling too far hurts and which game rules to use.", 0, 0, DF_FORCE_FALLINGST, false, {
			{LStr("OPTVAL_OFF", "Off"), 0},
			{LStr("OPTVAL_OLD", "Old"), DF_FORCE_FALLINGZD},
			{LStr("OPTVAL_HEXEN", "Hexen"), DF_FORCE_FALLINGHX},
			{LStr("OPTVAL_STRIFE", "Strife"), DF_FORCE_FALLINGST}
		}},
		{LStr("GMPLYMNU_SAMEMAP", "Same map"), "Restart the same map when someone exits (DM).", 0, DF_SAME_LEVEL, 0, false},
		{LStr("GMPLYMNU_SPAWNFARTHEST", "Spawn farthest"), "Spawn players as far as possible from other players (DM).", 0, DF_SPAWN_FARTHEST, 0, false},
		{LStr("GMPLYMNU_FORCERESPAWN", "Force respawn"), "Automatically respawn dead players after delay (DM).", 0, DF_FORCE_RESPAWN, 0, false},
		{LStr("GMPLYMNU_ALLOWARMOR", "Allow armor"), "Do not spawn armor (DM) when unchecked.", 0, DF_NO_ARMOR, 0, true},
		{LStr("GMPLYMNU_ALLOWEXIT", "Allow exit"), "Kill anyone who tries to exit the level (DM) when unchecked.", 0, DF_NO_EXIT, 0, true},
		{LStr("GMPLYMNU_INFINITEAMMO", "Infinite ammo"), "Don't use up ammo when firing.", 0, DF_INFINITE_AMMO, 0, false},
		{LStr("GMPLYMNU_NOMONSTERS", "No monsters"), "Don't spawn monsters.", 0, DF_NO_MONSTERS, 0, false},
		{LStr("GMPLYMNU_MONSTERSRESPAWN", "Monsters respawn"), "Monsters respawn sometime after their death.", 0, DF_MONSTERS_RESPAWN, 0, false},
		{LStr("GMPLYMNU_ITEMSRESPAWN", "Items respawn"), "Items other than invulnerability and invisibility respawn.", 0, DF_ITEMS_RESPAWN, 0, false},
		{LStr("GMPLYMNU_FASTMONSTERS", "Fast monsters"), "Monsters move and attack faster.", 0, DF_FAST_MONSTERS, 0, false},
		{LStr("GMPLYMNU_ALLOWJUMP", "Allow jump"), "Control whether players are allowed to jump (Default uses MAPINFO/game default).", 0, 0, DF_NO_JUMP | DF_YES_JUMP, false, {
			{LStr("OPTVAL_DEFAULT", "Default"), 0},
			{LStr("OPTVAL_OFF", "Off"), DF_NO_JUMP},
			{LStr("OPTVAL_ON", "On"), DF_YES_JUMP}
		}},
		{LStr("GMPLYMNU_ALLOWFREELOOK", "Allow freelook"), "Control whether players are allowed to look up and down (Default uses MAPINFO/game default).", 0, 0, DF_NO_FREELOOK | DF_YES_FREELOOK, false, {
			{LStr("OPTVAL_DEFAULT", "Default"), 0},
			{LStr("OPTVAL_OFF", "Off"), DF_NO_FREELOOK},
			{LStr("OPTVAL_ON", "On"), DF_YES_FREELOOK}
		}},
		{LStr("GMPLYMNU_ALLOWFOV", "Allow FOV"), "Only let the arbitrator set FOV when unchecked.", 0, DF_NO_FOV, 0, true},
		{LStr("GMPLYMNU_MULTIPLAYERWEAPONS", "Spawn multiplayer weapons"), "Don't spawn multiplayer weapons in coop when unchecked.", 0, DF_NO_COOP_WEAPON_SPAWN, 0, true},
		{LStr("GMPLYMNU_ALLOWCROUCH", "Allow crouch"), "Control whether players are allowed to crouch (Default uses MAPINFO/game default).", 0, 0, DF_NO_CROUCH | DF_YES_CROUCH, false, {
			{LStr("OPTVAL_DEFAULT", "Default"), 0},
			{LStr("OPTVAL_OFF", "Off"), DF_NO_CROUCH},
			{LStr("OPTVAL_ON", "On"), DF_YES_CROUCH}
		}},
		{LStr("GMPLYMNU_LOSEINVENTORY", "Lose entire inventory"), "Lose all inventory when respawning in coop.", 0, DF_COOP_LOSE_INVENTORY, 0, false},
		{LStr("GMPLYMNU_KEEPKEYS", "Keep keys"), "Lose keys when respawning in coop when unchecked.", 0, DF_COOP_LOSE_KEYS, 0, true},
		{LStr("GMPLYMNU_KEEPWEAPONS", "Keep weapons"), "Lose weapons when respawning in coop when unchecked.", 0, DF_COOP_LOSE_WEAPONS, 0, true},
		{LStr("GMPLYMNU_KEEPARMOR", "Keep armor"), "Lose armor when respawning in coop when unchecked.", 0, DF_COOP_LOSE_ARMOR, 0, true},
		{LStr("GMPLYMNU_KEEPPOWERUPS", "Keep powerups"), "Lose powerups when respawning in coop when unchecked.", 0, DF_COOP_LOSE_POWERUPS, 0, true},
		{LStr("GMPLYMNU_KEEPAMMO", "Keep ammo"), "Lose ammo when respawning in coop when unchecked.", 0, DF_COOP_LOSE_AMMO, 0, true},
		{LStr("GMPLYMNU_LOSEHALFAMMO", "Lose half ammo"), "Lose half ammo when respawning in coop (never below starting amount).", 0, DF_COOP_HALVE_AMMO, 0, false},
		{LStr("GMPLYMNU_INSTANTREACTION", "Monsters Nightmare Reflexes"), "Monsters react immediately upon sight (without fast movement).", 0, DF_INSTANT_REACTION, 0, false},
		{LStr("GMPLYMNU_DROPWEAPON", "Drop weapon"), "Players drop their current weapon upon death.", 1, DF2_YES_WEAPONDROP, 0, false},
		{LStr("GMPLYMNU_NOTEAMSWITCH", "No team switching"), "Prevent players from changing teams.", 1, DF2_NO_TEAM_SWITCH, 0, false},
		{LStr("GMPLYMNU_DOUBLEAMMO", "Double ammo"), "Double ammo received from pickups.", 1, DF2_YES_DOUBLEAMMO, 0, false},
		{LStr("GMPLYMNU_DEGENERATION", "Degeneration"), "Slowly lose health over 100%.", 1, DF2_YES_DEGENERATION, 0, false},
		{LStr("GMPLYMNU_BFGFREEAIM", "Allow BFG aiming"), "Prevent BFG vertical freeaim when unchecked.", 1, DF2_NO_FREEAIMBFG, 0, true},
		{LStr("GMPLYMNU_BARRELSRESPAWN", "Barrels respawn"), "Explosive barrels respawn after detonation.", 1, DF2_BARRELS_RESPAWN, 0, false},
		{LStr("GMPLYMNU_RESPAWNPROTECTION", "Respawn protection"), "Temporary invulnerability upon respawn (DM).", 1, DF2_YES_RESPAWN_INVUL, 0, false},
		{LStr("GMPLYMNU_SPAWNWHEREDIED", "Spawn where died"), "Coop players respawn at their death location.", 1, DF2_SAME_SPAWN_SPOT, 0, false},
		{LStr("GMPLYMNU_KEEPFRAGS", "Keep frags gained"), "Preserve frag counts across level changes (DM).", 1, DF2_YES_KEEPFRAGS, 0, false},
		{LStr("LAUNCHER_NO_PLAYER_RESPAWN", "No respawn (players)"), "Players cannot respawn after dying.", 1, DF2_NO_RESPAWN, 0, false},
		{LStr("GMPLYMNU_LOSEFRAG", "Lose frag if fragged"), "Lose a frag when killed by another player (DM).", 1, DF2_YES_LOSEFRAG, 0, false},
		{LStr("GMPLYMNU_INFINITEINVENTORY", "Infinite inventory"), "Inventory items are not consumed on use.", 1, DF2_INFINITE_INVENTORY, 0, false},
		{LStr("GMPLYMNU_NOMONSTERSTOEXIT", "No monsters to exit"), "All monsters must be killed before exiting the level.", 1, DF2_KILL_MONSTERS, 0, false},
		{LStr("GMPLYMNU_ALLOWAUTOMAP", "Allow automap"), "Disallow viewing the automap when unchecked.", 1, DF2_NO_AUTOMAP, 0, true},
		{LStr("GMPLYMNU_AUTOMAPALLIES", "Automap allies"), "Hide allies from automap when unchecked.", 1, DF2_NO_AUTOMAP_ALLIES, 0, true},
		{LStr("GMPLYMNU_ALLOWSPYING", "Allow spying"), "Disallow spying on allies when unchecked.", 1, DF2_DISALLOW_SPYING, 0, true},
		{LStr("GMPLYMNU_CHASECAM", "Chasecam cheat"), "Allow players to use chasecam in deathmatch without sv_cheats.", 1, DF2_CHASECAM, 0, false},
		{LStr("GMPLYMNU_ALLOWSUICIDE", "Allow Suicide"), "Disallow suicide command when unchecked.", 1, DF2_NOSUICIDE, 0, true},
		{LStr("GMPLYMNU_NOAUTOAIM", "Allow Autoaim"), "Disallow autoaim when unchecked.", 1, DF2_NOAUTOAIM, 0, true},
		{LStr("GMPLYMNU_DONTCHECKAMMO", "Check ammo for weapon switch"), "Skip ammo check when switching weapons when unchecked.", 1, DF2_DONTCHECKAMMO, 0, true},
		{LStr("GMPLYMNU_KILLBOSSSPAWNS", "Icon's death kills its spawns"), "Killing Icon of Sin removes boss-cube spawned monsters.", 1, DF2_KILLBOSSMONST, 0, false},
		{LStr("GMPLYMNU_NOCOUNTENDMONSTER", "End sector counts for kill %"), "Ignore monsters in 'end level when dying' sectors when unchecked.", 1, DF2_NOCOUNTENDMONST, 0, true},
		{LStr("GMPLYMNU_SUPERRESPAWN", "Big powerups respawn"), "Invulnerability and invisibility respawn.", 1, DF2_RESPAWN_SUPER, 0, false},
		{LStr("GMPLYMNU_MULTIPLAYERTHINGS", "Spawn multiplayer things in coop"), "Don't spawn multiplayer-only things in coop when unchecked.", 1, DF2_NO_COOP_THING_SPAWN, 0, true},
		{LStr("GMPLYMNU_ALWAYSSPAWNMULTI", "Always spawn multiplayer objects"), "Always spawn multiplayer objects, even in singleplayer.", 1, DF2_ALWAYS_SPAWN_MULTI, 0, false},
		{LStr("GMPLYMNU_NOEXTRAAMMO", "Weapons give extra ammo (DM)"), "Disable extra ammo when picking up duplicate weapons when unchecked.", 1, DF2_NO_EXTRA_AMMO, 0, true},
		{LStr("GMPLYMNU_NOPLAYERCLIP", "Move and shoot through other players"), "Players can walk and shoot through each other.", 2, DF3_NO_PLAYER_CLIP, 0, false},
		{LStr("GMPLYMNU_SHAREKEYS", "Share keys with all players"), "Keys picked up are shared with all coop players.", 2, DF3_COOP_SHARE_KEYS, 0, false},
		{LStr("GMPLYMNU_LOCALITEMS", "Players pick up their own copy of items in multiplayer"), "In coop, each player picks up their own copy of items instead of taking them for everyone.", 2, DF3_LOCAL_ITEMS, 0, false},
		{LStr("GMPLYMNU_NOLOCALDROP", "Disable client-side pick ups on dropped items"), "Dropped items from actors are not picked up locally even with Local item pickups enabled.", 2, DF3_NO_LOCAL_DROPS, 0, false},
		{LStr("GMPLYMNU_COOPITEMS", "Spawn co-op only pickup items in multiplayer"), "Disable co-op only inventory items in coop when unchecked.", 2, DF3_NO_COOP_ONLY_ITEMS, 0, true},
		{LStr("GMPLYMNU_COOPTHINGS", "Spawn co-op only things in multiplayer"), "Disable co-op only actors in coop when unchecked.", 2, DF3_NO_COOP_ONLY_THINGS, 0, true},
		{LStr("GMPLYMNU_REMEMBERWEAP", "Remember selected weapon on respawn"), "Keep last used weapon out when respawning in coop.", 2, DF3_REMEMBER_LAST_WEAP, 0, false},
		{LStr("GMPLYMNU_PISTOLSTART", "Pistol start every map"), "Reset player inventory at each level transition.", 2, DF3_PISTOL_START, 0, false},
	};
}

std::vector<FlagInfo> getCompatFlagsList()
{
	return {
		{LStr("CMPTMNU_SHORTTEX", "Find shortest textures like Doom"), "Use Doom's shortest texture calculation.", 0, COMPATF_SHORTTEX, 0, false},
		{LStr("CMPTMNU_STAIRS", "Use buggier stair building"), "Preserve original Doom stair building index bug.", 0, COMPATF_STAIRINDEX, 0, false},
		{LStr("CMPTMNU_LIMITPAIN", "Limit Pain Elementals' Lost Souls"), "Prevent Pain Elementals from spawning above 20 Lost Souls.", 0, COMPATF_LIMITPAIN, 0, false},
		{LStr("CMPTMNU_SILENTPICKUP", "Don't let others hear your pickups"), "Item pickup sounds are heard only locally.", 0, COMPATF_SILENTPICKUP, 0, false},
		{LStr("CMPTMNU_NOPASSOVER", "Actors are infinitely tall"), "Use infinitely tall actor collision.", 0, COMPATF_NO_PASSMOBJ, 0, false},
		{LStr("CMPTMNU_SOUNDSLOTS", "Cripple sound for silent BFG trick"), "Limit actors to one sound channel at a time.", 0, COMPATF_MAGICSILENCE, 0, false},
		{LStr("CMPTMNU_WALLRUN", "Enable wall running"), "Enable classic Doom wall-running physics.", 0, COMPATF_WALLRUN, 0, false},
		{LStr("CMPTMNU_NOTOSSDROPS", "Spawn item drops on the floor"), "Spawn dropped items on the floor instead of tossing.", 0, COMPATF_NOTOSSDROPS, 0, false},
		{LStr("CMPTMNU_USEBLOCKING", "All special lines can block <use>"), "Allow any special line to block use activation.", 0, COMPATF_USEBLOCKING, 0, false},
		{LStr("CMPTMNU_NODOORLIGHT", "Disable BOOM door light effect"), "Disable BOOM gradual door sector lighting.", 0, COMPATF_NODOORLIGHT, 0, false},
		{LStr("CMPTMNU_RAVENSCROLL", "Raven scrollers use original speed"), "Use original carrying speed for Heretic/Hexen scrollers.", 0, COMPATF_RAVENSCROLL, 0, false},
		{LStr("CMPTMNU_SOUNDTARGET", "Use original sound target handling"), "Use classic Doom sector sound target rules.", 0, COMPATF_SOUNDTARGET, 0, false},
		{LStr("CMPTMNU_DEHHEALTH", "DEH health settings like Doom2.exe"), "Cap Dehacked max health like Doom2.exe.", 0, COMPATF_DEHHEALTH, 0, false},
		{LStr("CMPTMNU_TRACE", "Self ref. sectors don't block shots"), "Trace ignores lines with same sector on both sides.", 0, COMPATF_TRACE, 0, false},
		{LStr("CMPTMNU_DROPOFF", "Monsters get stuck over dropoffs"), "Monsters cannot step away while hanging over dropoffs.", 0, COMPATF_DROPOFF, 0, false},
		{LStr("CMPTMNU_BOOMSCROLL", "Boom scrollers are additive"), "Additive scrolling sectors like BOOM.", 0, COMPATF_BOOMSCROLL, 0, false},
		{LStr("CMPTMNU_INVISIBILITY", "Monsters see invisible players"), "Use original Doom visibility behavior for invisibility.", 0, COMPATF_INVISIBILITY, 0, false},
		{LStr("CMPTMNU_SILENTINSTANTFLOORS", "Inst. moving floors are not silent"), "Play movement sound on instant floor moves.", 0, COMPATF_SILENT_INSTANT_FLOORS, 0, false},
		{LStr("CMPTMNU_SECTORSOUNDS", "Sector sounds use center as source"), "Use sector center instead of closest point for sound origin.", 0, COMPATF_SECTORSOUNDS, 0, false},
		{LStr("CMPTMNU_MISSILECLIP", "Use Doom heights for missile clipping"), "Use original projectile clipping heights.", 0, COMPATF_MISSILECLIP, 0, false},
		{LStr("CMPTMNU_CROSSDROPOFF", "Monsters cannot cross dropoffs"), "Prevent monsters from walking or being pushed over tall ledges (Doom behavior).", 0, COMPATF_CROSSDROPOFF, 0, false},
		{LStr("CMPTMNU_ANYBOSSDEATH", "Allow any bossdeath for level special"), "Any monster calling A_BossDeath triggers specials.", 0, COMPATF_ANYBOSSDEATH, 0, false},
		{LStr("CMPTMNU_MINOTAUR", "No Minotaur floor flames in water"), "Minotaur floor flames fail to spawn when feet are clipped in water/liquid.", 0, COMPATF_MINOTAUR, 0, false},
		{LStr("CMPTMNU_MUSHROOM", "Original A_Mushroom speed in DEH mods"), "Use original velocity calculation for A_Mushroom.", 0, COMPATF_MUSHROOM, 0, false},
		{LStr("CMPTMNU_MBFMONSTERMOVE", "Monster movement is affected by effects"), "MBF monster movement is affected by sector friction and pushers/pullers.", 0, COMPATF_MBFMONSTERMOVE, 0, false},
		{LStr("CMPTMNU_VILEGHOSTS", "Crushed monsters get resurrected as ghosts"), "Enable classic Arch-Vile ghost bug on crushed corpses.", 0, COMPATF_VILEGHOSTS, 0, false},
		{LStr("CMPTMNU_NOBLOCKFRIENDS", "Friendly monsters aren't blocked"), "Friendly monsters ignore monster-blocking lines.", 0, COMPATF_NOBLOCKFRIENDS, 0, false},
		{LStr("CMPTMNU_SPRITESORT", "Invert sprite sorting"), "Use original sprite sorting order for equal distances.", 0, COMPATF_SPRITESORT, 0, false},
		{LStr("CMPTMNU_HITSCAN", "Use Doom code for hitscan checks"), "Original blockmap and hit check rules for hitscans.", 0, COMPATF_HITSCAN, 0, false},
		{LStr("CMPTMNU_LIGHT", "Find neighboring light like Doom"), "Use original lowest-neighboring light search.", 0, COMPATF_LIGHT, 0, false},
		{LStr("CMPTMNU_POLYOBJ", "Draw polyobjects like Hexen"), "Link polyobjects to their center subsector instead of BSP splitting (old ZDoom/Hexen behavior).", 0, COMPATF_POLYOBJ, 0, false},
		{LStr("CMPTMNU_MASKEDMIDTEX", "Ignore Y offsets on masked midtextures"), "Render masked midtextures ignoring vertical offsets.", 0, COMPATF_MASKEDMIDTEX, 0, false},
		{LStr("CMPTMNU_BADANGLES", "Cannot travel straight NSEW"), "Reproduce vanilla Doom's imprecise spawn/teleport angle so players cannot move strictly NSEW.", 1, COMPATF2_BADANGLES, 0, false},
		{LStr("CMPTMNU_FLOORMOVE", "Use Doom's floor motion behavior"), "Use original Doom floor movement checks.", 1, COMPATF2_FLOORMOVE, 0, false},
		{LStr("CMPTMNU_SOUNDCUTOFF", "Sounds stop when actor vanishes"), "Cut off sounds when an actor is destroyed.", 1, COMPATF2_SOUNDCUTOFF, 0, false},
		{LStr("CMPTMNU_POINTONLINE", "Use Doom's point-on-line algorithm"), "Use original Doom point-on-line side test.", 1, COMPATF2_POINTONLINE, 0, false},
		{LStr("CMPTMNU_MULTIEXIT", "Level exit can be triggered more than once"), "Allow level exit specials to trigger repeatedly.", 1, COMPATF2_MULTIEXIT, 0, false},
		{LStr("CMPTMNU_TELEPORT", "Scripted teleports don't trigger sector actions"), "Prevent indirect/scripted teleports from triggering sector actions.", 1, COMPATF2_TELEPORT, 0, false},
		{LStr("CMPTMNU_PUSHWINDOW", "Non-blocking lines can be pushed"), "Allow pushing two-sided non-blocking special lines (disables CheckForPushSpecial window check).", 1, COMPATF2_PUSHWINDOW, 0, false},
		{LStr("CMPTMNU_CHECKSWITCHRANGE", "Enable buggy CheckSwitchRange behavior"), "Use older switch vertical range check.", 1, COMPATF2_CHECKSWITCHRANGE, 0, false},
		{LStr("CMPTMNU_EXPLODE1", "No vertical thrust from explosions"), "Disable Z thrust from explosions.", 1, COMPATF2_EXPLODE1, 0, false},
		{LStr("CMPTMNU_EXPLODE2", "Use original Doom explosion behavior"), "Use classic explosion thrust behavior.", 1, COMPATF2_EXPLODE2, 0, false},
		{LStr("CMPTMNU_RAILINGHACK", "Enable buggy Strife railing"), "Emulate classic Strife railing stepping.", 1, COMPATF2_RAILING, 0, false},
		{LStr("CMPTMNU_SCRIPTWAIT", "Old ScriptWait implementation"), "ScriptWait doesn't wait on a non-running script.", 1, COMPATF2_SCRIPTWAIT, 0, false},
		{LStr("CMPTMNU_AVOIDHAZARDS", "Monsters avoid hazards (MBF)"), "Enable MBF monster hazard avoidance.", 1, COMPATF2_AVOID_HAZARDS, 0, false},
		{LStr("CMPTMNU_STAYONLIFT", "Monsters stay on lifts (MBF)"), "Keep monsters on moving lifts as in MBF.", 1, COMPATF2_STAYONLIFT, 0, false},
		{LStr("CMPTMNU_VOODOOZOMBIES", "Allow creation of zombie players"), "Killing a voodoo doll lowers player health without killing the real player (zombie player glitch).", 1, COMPATF2_VOODOO_ZOMBIES, 0, false},
		{LStr("CMPTMNU_FDTELEPORT", "Ignore floor Z when teleporting"), "Emulate Final Doom's teleporter height bug (ignore destination floor Z).", 1, COMPATF2_FDTELEPORT, 0, false},
		{LStr("CMPTMNU_NOACSARGCHECK", "Disable ACS arg count check"), "Skip strict ACS argument validation.", 1, COMPATF2_NOACSARGCHECK, 0, false},
		{LStr("CMPTMNU_NOVDOLLLOCKMSG", "Voodoo dolls suppress lock messages"), "Voodoo dolls don't print locked-door messages.", 1, COMPATF2_NOVDOLLLOCKMSG, 0, false},
		{LStr("CMPTMNU_EMULATEMIKOPORTALS", "Emulate Mikoportals"), "Emulate Z underflow below -32768 map units for Mikoportal setups.", 1, COMPATF2_EMULATEMIKOPORTALS, 0, false},
		{LStr("CMPTMNU_TRANSFERSECRET", "BOOM's floor changing special transfers secret."), "Allow BOOM floor-changing specials to transfer sector secret flags.", 1, COMPATF2_TRANSFERSECRET, 0, false},
		{"Disable reserved linedef flags", "Disable certain modern linedef flag features that may clash with older Doom maps.", 1, COMPATF2_RESERVEDLINEFLAG, 0, false},
	};
}
