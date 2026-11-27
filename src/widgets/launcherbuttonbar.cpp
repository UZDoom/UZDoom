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

#include <zwidget/widgets/pushbutton/pushbutton.h>

#include "gstrings.h"
#include "launcherbuttonbar.h"
#include "launcherwindow.h"
#include "themedata.h"

LauncherButtonbar::LauncherButtonbar(LauncherWindow* parent) : Widget(parent)
{
	PlayButton = new PushButton(this);
	ExitButton = new PushButton(this);

	Colorf accent = Theme::getAccent();
	PlayButton->SetStyleColor("background-color", accent);
	PlayButton->SetStyleColor("color", Colorf::fromRgb(0xFFFFFF));
	PlayButton->SetStyleColor("border-left-color", accent);
	PlayButton->SetStyleColor("border-top-color", accent);
	PlayButton->SetStyleColor("border-right-color", accent);
	PlayButton->SetStyleColor("border-bottom-color", accent);

	PlayButton->OnClick = [this]() { OnPlayButtonClicked(); };
	ExitButton->OnClick = [this]() { OnExitButtonClicked(); };
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
}

double LauncherButtonbar::GetPreferredHeight()
{
	return 24.0 + std::max(PlayButton->GetPreferredHeight(), ExitButton->GetPreferredHeight());
}

void LauncherButtonbar::OnGeometryChanged()
{
	double w, h;
	h = std::max(PlayButton->GetPreferredHeight(), ExitButton->GetPreferredHeight());
	w = std::max(116.0, 16.0 + std::max(PlayButton->GetPreferredWidth(), ExitButton->GetPreferredWidth()));
	PlayButton->SetFrameGeometry(20.0, 10.0, w, h);
	ExitButton->SetFrameGeometry(GetWidth() - 20.0 - w, 10.0, w, h);
}

void LauncherButtonbar::OnPlayButtonClicked()
{
	GetLauncher()->Start();
}

void LauncherButtonbar::OnExitButtonClicked()
{
	GetLauncher()->Exit();
}

LauncherWindow* LauncherButtonbar::GetLauncher() const
{
	return static_cast<LauncherWindow*>(Parent());
}
