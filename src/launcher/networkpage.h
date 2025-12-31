/*
** networkpage.h
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
#include "gstrings.h"

class LauncherWindow;
class CheckboxLabel;
class LineEdit;
class Dropdown;
class TextLabel;
class PushButton;
class TabWidget;
struct WadStuff;
struct FStartupSelectionInfo;

class HostSubPage;
class JoinSubPage;

class NetworkPage : public Widget
{
public:
	NetworkPage(LauncherWindow* launcher, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void UpdatePlayButton();
	bool IsInHost() const;
	void SetValues(FStartupSelectionInfo& info) const;
	void InitializeTabs(const FStartupSelectionInfo& info);

private:
	void OnGeometryChanged() override;
	void OnSetFocus() override;
	void OnIWADsListActivated();

	LauncherWindow* Launcher = nullptr;

	Dropdown* IWADsDropdown = nullptr;
	TextLabel* SaveFileLabel = nullptr;
	LineEdit* SaveFileEdit = nullptr;
	CheckboxLabel* SaveFileCheckbox = nullptr;
	
	HostSubPage* HostPage = nullptr;
	JoinSubPage* JoinPage = nullptr;
	TabWidget* StartPages = nullptr;
	
	TextLabel* ParametersLabel = nullptr;
	LineEdit* ParametersEdit = nullptr;
	CheckboxLabel* SaveParametersCheckbox = nullptr;
	TextLabel* PlayerClassLabel = nullptr;
	LineEdit* PlayerClassEdit = nullptr;
};

class HostSubPage : public Widget
{
public:
	HostSubPage(NetworkPage* main, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void SetValues(FStartupSelectionInfo& info) const;

private:
	void OnGeometryChanged() override;

	NetworkPage* MainTab = nullptr;

	TextLabel* TicDupLabel = nullptr;
	Dropdown* TicDupDropdown = nullptr;
	CheckboxLabel* ExtraTicCheckbox = nullptr;

	TextLabel* GameModesLabel = nullptr;
	Dropdown* GameModesDropdown = nullptr;
	CheckboxLabel* AltDeathmatchCheckbox = nullptr;
	TextLabel* TeamLabel = nullptr;
	LineEdit* TeamEdit = nullptr;
	TextLabel* TeamHintLabel = nullptr;

	TextLabel* MaxPlayersLabel = nullptr;
	LineEdit* MaxPlayersEdit = nullptr;
	TextLabel* MaxPlayerHintLabel = nullptr;
	TextLabel* PortLabel = nullptr;
	LineEdit* PortEdit = nullptr;
	TextLabel* PortHintLabel = nullptr;
};

class JoinSubPage : public Widget
{
public:
	JoinSubPage(NetworkPage* main, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void SetValues(FStartupSelectionInfo& info) const;

private:
	void OnGeometryChanged() override;

	NetworkPage* MainTab = nullptr;

	TextLabel* AddressLabel = nullptr;
	LineEdit* AddressEdit = nullptr;
	TextLabel* AddressPortLabel = nullptr;
	LineEdit* AddressPortEdit = nullptr;
	TextLabel* AddressPortHintLabel = nullptr;
	
	TextLabel* TeamDeathmatchLabel = nullptr;
	TextLabel* TeamLabel = nullptr;
	LineEdit* TeamEdit = nullptr;
	TextLabel* TeamHintLabel = nullptr;
};
