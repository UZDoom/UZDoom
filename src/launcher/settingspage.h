/*
** settingspage.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

#define RENDER_BACKENDS

class LauncherWindow;
class TextLabel;
class CheckboxLabel;
class ListView;
class Dropdown;
struct FStartupSelectionInfo;

class SettingsPage : public Widget
{
public:
	SettingsPage(LauncherWindow* launcher, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void SetValues(FStartupSelectionInfo& info) const;

private:
	void OnLanguageChanged(int i);
	void OnGeometryChanged() override;

	LauncherWindow* Launcher = nullptr;

	TextLabel* LangLabel = nullptr;
	TextLabel* GeneralLabel = nullptr;
	TextLabel* ExtrasLabel = nullptr;
	TextLabel* LoadLabel = nullptr;
	CheckboxLabel* FullscreenCheckbox = nullptr;
	CheckboxLabel* DisableAutoloadCheckbox = nullptr;
	CheckboxLabel* DontAskAgainCheckbox = nullptr;
	CheckboxLabel* LightsCheckbox = nullptr;
	CheckboxLabel* BrightmapsCheckbox = nullptr;
	CheckboxLabel* WidescreenCheckbox = nullptr;
	CheckboxLabel* SupportWadsCheckbox = nullptr;
#ifdef RENDER_BACKENDS
	TextLabel* BackendLabel = nullptr;
	CheckboxLabel* VulkanCheckbox = nullptr;
	CheckboxLabel* OpenGLCheckbox = nullptr;
	CheckboxLabel* GLESCheckbox = nullptr;
#endif
	ListView* LangList = nullptr;
	Dropdown* LoadList = nullptr;

	TArray<std::pair<FString, FString>> languages;
	bool hideLanguage = false;

	int ExtraWadFlags = 0;
};
