/*
** starter.cpp
**
** The Qt6 Laucher STARTS HERE
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

#include "starter.h"
#include "about.h"
#include "const.h"
#include "launcherMainWindow.h"
#include "loader.h"
#include "profile.h"

#ifdef HAS_UPDATER
#include "curl_loader.h"
#include "updater.h"
#endif

#include "gstrings.h"
#include "i_interface.h"
#include "i_specialpaths.h"
#include "version.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <algorithm>
#include <memory>
#include <vector>

namespace
{
	std::string engineThemeIdToName(int themeId)
	{
		if (themeId == 1)
			return "dark";
		if (themeId == 2)
			return "light";
		return "auto";
	}

	int themeNameToEngineThemeId(const std::string& themeName)
	{
		switch (parseThemeName(themeName))
		{
		case LauncherTheme::Light:
			return 2;
		case LauncherTheme::Dark:
		case LauncherTheme::HighContrast:
			return 1;
		case LauncherTheme::Auto:
		default:
			return 0;
		}
	}
}

bool Starter::ExecModal(FStartupSelectionInfo& info)
{
#ifdef HAS_UPDATER
	LoadCurl();
#endif

	QApplication* app = ensureLauncherQtApplication();

	const QString appDataRoot = QDir::cleanPath(QString::fromUtf8(M_GetAppDataPath(true).GetChars()));
	const QString launcherRoot = QDir(appDataRoot).filePath("launcher");
	const QString profilesRoot = QDir(launcherRoot).filePath("profiles");
	QDir().mkpath(launcherRoot);
	QDir().mkpath(profilesRoot);

	LauncherConst::ROOT_DIR = QDir::cleanPath(launcherRoot).toStdString();
	LauncherConst::PROFILE_DIR = QDir::cleanPath(profilesRoot).toStdString();
	LauncherConst::CONFIG_FILE = QDir(launcherRoot).filePath("config.json").toStdString();
	LauncherConst::EXEC_DIR = QDir::cleanPath(QCoreApplication::applicationDirPath()).toStdString();

	g_discoveredEngineIWads.clear();
	if (info.Wads)
	{
		g_discoveredEngineIWads.reserve(info.Wads->Size());
		for (unsigned i = 0; i < info.Wads->Size(); ++i)
		{
			DiscoveredIWad item;
			item.name = (*info.Wads)[i].Name.GetChars();
			item.path = (*info.Wads)[i].Path.GetChars();
			g_discoveredEngineIWads.push_back(std::move(item));
		}
	}

	const bool firstLauncherConfig = !QFileInfo::exists(QString::fromStdString(LauncherConst::CONFIG_FILE));
	LauncherConfig cfg = loadLauncherConfig();
	if (firstLauncherConfig)
	{
		if (info.DefaultLanguage.IsNotEmpty())
			cfg.language = info.DefaultLanguage.GetChars();
		cfg.theme = engineThemeIdToName(info.DefaultPreferredTheme);
		cfg.showReleaseNotes = info.notifyNewRelease;
		cfg.maximized = info.LauncherMaximized;
		saveLauncherConfig(cfg);
	}

	if (!cfg.language.empty())
		GStrings.UpdateLanguage(cfg.language.c_str());

	applyLauncherTheme(cfg.theme);

	std::vector<Profile> profiles = loadAllProfiles();

	{
		const bool showUpcoming = (info.displayRelease >= 3);
		LauncherMainWindow mainWin(cfg, std::move(profiles), showUpcoming);
		if (info.LauncherWidth >= 780 && info.LauncherHeight >= 520)
			mainWin.resize(static_cast<int>(info.LauncherWidth), static_cast<int>(info.LauncherHeight));

		if (cfg.maximized)
			mainWin.showMaximized();
		else
			mainWin.show();

		QTimer::singleShot(0, &mainWin, [&mainWin]() {
			mainWin.startStartupIWadDetection();
		});

		if (info.displayRelease && info.notifyNewRelease && cfg.showReleaseNotes)
		{
			QTimer::singleShot(0, &mainWin, [&mainWin, showUpcoming]() {
				bool showAfter = mainWin.config().showReleaseNotes;
				AboutDialog::showAdditionalInfo(showAfter, true, &mainWin, {}, showUpcoming);
				mainWin.setShowReleaseNotes(showAfter);
			});
		}

#ifdef HAS_UPDATER
		if (IsCurlLoaded())
		{
			QTimer::singleShot(0, &mainWin, [&mainWin]() {
				mainWin.startStartupUpdateCheck();
			});
		}
#endif

		const bool prevQuitOnClose = QApplication::quitOnLastWindowClosed();
		QApplication::setQuitOnLastWindowClosed(true);
		app->exec();
		QApplication::setQuitOnLastWindowClosed(prevQuitOnClose);

		const LauncherConfig& finalCfg = mainWin.config();
		info.notifyNewRelease = finalCfg.showReleaseNotes;
		info.DefaultLanguage = finalCfg.language.c_str();
		info.DefaultPreferredTheme = themeNameToEngineThemeId(finalCfg.theme);
		const QRect normalGeom = mainWin.isMaximized() ? mainWin.normalGeometry() : mainWin.geometry();
		if (normalGeom.width() >= 780 && normalGeom.height() >= 520)
		{
			info.LauncherWidth = static_cast<unsigned>(normalGeom.width());
			info.LauncherHeight = static_cast<unsigned>(normalGeom.height());
		}
		else
		{
			info.LauncherWidth = static_cast<unsigned>(std::max(0, mainWin.width()));
			info.LauncherHeight = static_cast<unsigned>(std::max(0, mainWin.height()));
		}
		info.LauncherMaximized = finalCfg.maximized;
#ifdef HAS_UPDATER
		if (IsCurlLoaded())
			syncStartupUpdaterInfo(info);
#endif
		info.SaveInfo();
	}

	return false;
}
