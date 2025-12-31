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
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <zwidget/core/widget.h>
#include "tarray.h"
#include "zstring.h"

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

	void Start();
	void Exit();
	bool IsInMultiplayer() const;
	bool IsHosting() const;
	void UpdatePlayButton();

private:
	void OnClose() override;
	void OnGeometryChanged() override;

	LauncherBanner* Banner = nullptr;
	TabWidget* Pages = nullptr;
	LauncherButtonbar* Buttonbar = nullptr;

	PlayGamePage* PlayGame = nullptr;
	SettingsPage* Settings = nullptr;
	NetworkPage* Network = nullptr;
	ReleasePage* Release = nullptr;
	AboutPage* About = nullptr;

	FStartupSelectionInfo* Info = nullptr;

	bool ExecResult = false;

	friend AboutPage;
};
