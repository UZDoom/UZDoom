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
** SPDX-License-Identifier: GPL-3.0-or-later
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
