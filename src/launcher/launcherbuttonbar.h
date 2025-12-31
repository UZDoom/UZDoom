/*
** launcherbuttonbar.h
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

class LauncherWindow;
class PushButton;

class LauncherButtonbar : public Widget
{
public:
	LauncherButtonbar(LauncherWindow* parent);
	void UpdateLanguage();

	double GetPreferredHeight() const;

private:
	void OnGeometryChanged() override;
	void OnPlayButtonClicked();
	void OnExitButtonClicked();

	LauncherWindow* GetLauncher() const;

	PushButton* PlayButton = nullptr;
	PushButton* ExitButton = nullptr;
};
