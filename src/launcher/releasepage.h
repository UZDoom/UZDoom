/*
** releasepage.h
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
#include <rapidxml/rapidxml.hpp>

class LauncherWindow;
class CheckboxLabel;
class TextEdit;
struct FStartupSelectionInfo;

class ReleasePage : public Widget
{
public:
	ReleasePage(LauncherWindow* launcher, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void SetValues(FStartupSelectionInfo& info) const;

private:
	void OnLanguageChanged(int i);
	void OnGeometryChanged() override;

	LauncherWindow* Launcher = nullptr;

	TextEdit* Notes = nullptr;
	CheckboxLabel* ShowThis = nullptr;

	static FString _ParseReleaseNotes(rapidxml::xml_node<char> *);
	static FString _BuildReleaseNotes(rapidxml::xml_document<> &);
	static char * _OpenReleaseNotes();
	static FString GetReleaseNotes();
};
