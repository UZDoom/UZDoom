/*
** launcherwindow.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <zwidget/core/widget.h>
#include "tarray.h"
#include "zstring.h"

#ifdef HAS_UPDATER
class UpdateButtonBar;
#endif

class TabWidget;
class LauncherBanner;
class LauncherButtonbar;
class PlayGamePage;
class SettingsPage;
class NetworkPage;
class ReleasePage;
class AboutPage;
struct WadStuff;
struct FStartupSelectionInfo;

class LauncherWindow : public Widget
{
public:
	static bool ExecModal(FStartupSelectionInfo& info);

	LauncherWindow(FStartupSelectionInfo& info);
	void UpdateLanguage();

	void UpdateSize();

	void Start();
	void Exit();
	bool IsInMultiplayer() const;
	bool IsHosting() const;
	void UpdatePlayButton();
	void ForceCheckUpdate();

private:
	void OnClose() override;
	void OnGeometryChanged() override;
	void OnWindowClose() override;

	LauncherBanner* Banner = nullptr;
	TabWidget* Pages = nullptr;
	LauncherButtonbar* Buttonbar = nullptr;
#ifdef HAS_UPDATER
	UpdateButtonBar* UpdateBar = nullptr;
#endif

	PlayGamePage* PlayGame = nullptr;
	SettingsPage* Settings = nullptr;
	NetworkPage* Network = nullptr;
	ReleasePage* Release = nullptr;
	AboutPage* About = nullptr;

	FStartupSelectionInfo* Info = nullptr;

	bool ExecResult = false;

	double topHeight = 0.0f;

	friend AboutPage;
};
