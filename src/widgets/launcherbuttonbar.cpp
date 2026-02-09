/*
** launcherbuttonbar.cpp
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

#include "launcherbuttonbar.h"
#include "launcherwindow.h"
#include "gstrings.h"
#include <zwidget/widgets/pushbutton/pushbutton.h>

LauncherButtonbar::LauncherButtonbar(LauncherWindow* parent) : Widget(parent)
{
	PlayButton = new PushButton(this);
	ExitButton = new PushButton(this);
 EditorButton = new PushButton(this);

	PlayButton->OnClick = [=]() { OnPlayButtonClicked(); };
	ExitButton->OnClick = [=]() { OnExitButtonClicked(); };
 EditorButton->OnClick = [=]() { OnEditorButtonClicked(); };
}

void LauncherButtonbar::UpdateLanguage()
{
	auto launcher = GetLauncher();
	if (!launcher->IsInMultiplayer())
		PlayButton->SetText(GStrings.GetString("PICKER_PLAY"));
	else if (launcher->IsHosting())
		PlayButton->SetText(GStrings.GetString("PICKER_PLAYHOST"));
	else
		PlayButton->SetText(GStrings.GetString("PICKER_PLAYJOIN"));

	ExitButton->SetText(GStrings.GetString("PICKER_EXIT"));
 EditorButton->SetText("Editor");
}

double LauncherButtonbar::GetPreferredHeight() const
{
	return 20.0 + std::max(PlayButton->GetPreferredHeight(), std::max(ExitButton->GetPreferredHeight(), EditorButton->GetPreferredHeight()));
}

void LauncherButtonbar::OnGeometryChanged()
{
	PlayButton->SetFrameGeometry(20.0, 10.0, 120.0, PlayButton->GetPreferredHeight());
	ExitButton->SetFrameGeometry(GetWidth() - 20.0 - 120.0, 10.0, 120.0, ExitButton->GetPreferredHeight());
 EditorButton->SetFrameGeometry((GetWidth() - 120.0) * 0.5, 10.0, 120.0, EditorButton->GetPreferredHeight());
}

void LauncherButtonbar::OnPlayButtonClicked()
{
	GetLauncher()->Start();
}

void LauncherButtonbar::OnExitButtonClicked()
{
	GetLauncher()->Exit();
}

void LauncherButtonbar::OnEditorButtonClicked()
{
 GetLauncher()->OnEditorButtonClicked();
}

LauncherWindow* LauncherButtonbar::GetLauncher() const
{
	return static_cast<LauncherWindow*>(Parent());
}
