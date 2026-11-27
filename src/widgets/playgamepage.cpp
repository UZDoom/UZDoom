/*
** playgamepage.cpp
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

#include <zwidget/widgets/checkboxlabel/checkboxlabel.h>
#include <zwidget/widgets/lineedit/lineedit.h>
#include <zwidget/widgets/listview/listview.h>
#include <zwidget/widgets/textlabel/textlabel.h>

#include "gstrings.h"
#include "i_interface.h"
#include "launcherwindow.h"
#include "playgamepage.h"
#include "themedata.h"
#include "version.h"

PlayGamePage::PlayGamePage(LauncherWindow* launcher, const FStartupSelectionInfo& info) : Widget(nullptr), Launcher(launcher)
{
	Colorf mutedText = Theme::getMode() == DARK ? Colorf::fromRgb(0x98989D) : Colorf::fromRgb(0x6E6E73);
	WelcomeLabel = new TextLabel(this);
	VersionLabel = new TextLabel(this);
	VersionLabel->SetTextAlignment(TextLabelAlignment::Right);
	VersionLabel->SetStyleColor("color", mutedText);
	SelectLabel = new TextLabel(this);
	SelectLabel->SetStyleColor("color", mutedText);
	ParametersLabel = new TextLabel(this);
	ParametersLabel->SetStyleColor("color", mutedText);
	GamesList = new ListView(this);
	ParametersEdit = new LineEdit(this);
	SaveArgsCheckbox = new CheckboxLabel(this);

	SaveArgsCheckbox->SetChecked(info.bSaveArgs);
	if (!info.DefaultArgs.IsEmpty())
		ParametersEdit->SetText(info.DefaultArgs.GetChars());

	for (const auto& wad : *info.Wads)
	{
		const char* filepart = strrchr(wad.Path.GetChars(), '/');
		if (filepart == nullptr)
			filepart = wad.Path.GetChars();
		else
			++filepart;

		FString work;
		if (*filepart)
			work.Format("%s (%s)", wad.Name.GetChars(), filepart);
		else
			work = wad.Name.GetChars();

		GamesList->AddItem(work.GetChars());
	}

	if (info.DefaultIWAD >= 0 && info.DefaultIWAD < info.Wads->SSize())
	{
		GamesList->SetSelectedItem(info.DefaultIWAD);
	}

	GamesList->OnActivated = [this]() { OnGamesListActivated(); };
}

void PlayGamePage::SetValues(FStartupSelectionInfo& info) const
{
	info.DefaultIWAD = GamesList->GetSelectedItem();
	info.DefaultArgs = ParametersEdit->GetText();
	info.bSaveArgs = SaveArgsCheckbox->GetChecked();
}

void PlayGamePage::UpdateLanguage()
{
	SelectLabel->SetText(GStrings.GetString("PICKER_SELECT"));
	ParametersLabel->SetText(GStrings.GetString("PICKER_ADDPARM"));
	FString welcomeText = GStrings.GetString("PICKER_WELCOME");
	welcomeText.Substitute("%s", GAMENAME);
	WelcomeLabel->SetText(welcomeText.GetChars());
	FString versionText = GStrings.GetString("PICKER_VERSION");
	versionText.Substitute("%s", GetVersionString());
	VersionLabel->SetText(versionText.GetChars());
	SaveArgsCheckbox->SetText(GStrings.GetString("PICKER_REMPARM"));
}

void PlayGamePage::OnGamesListActivated()
{
	Launcher->Start();
}

void PlayGamePage::OnSetFocus()
{
	GamesList->SetFocus();
}

bool PlayGamePage::OnFileDrop(std::string path)
{
	auto text = ParametersEdit->GetText();
	if (!text.empty()) text += " ";

	std::string arg;
	auto ext = path.substr(path.length() - 4u);
	if (!stricmp(ext.c_str(), ".deh"))
		arg = "-deh";
	else if (!stricmp(ext.c_str(), ".bex"))
		arg = "-bex";
	else
		arg = "-file";

	text += arg;
	text += " \"";
	text += path;
	text += "\"";
	ParametersEdit->SetText(text);
	return true;
}

void PlayGamePage::OnGeometryChanged()
{
	double y = 12.0;

	const double halfW = GetWidth() * 0.5;
	WelcomeLabel->SetFrameGeometry(0.0, y, halfW, WelcomeLabel->GetPreferredHeight());
	VersionLabel->SetFrameGeometry(halfW, y, halfW, VersionLabel->GetPreferredHeight());

	y += VersionLabel->GetPreferredHeight() + 10.0;

	SelectLabel->SetFrameGeometry(0.0, y, GetWidth(), SelectLabel->GetPreferredHeight());
	y += SelectLabel->GetPreferredHeight() + 4.0;

	const double listViewTop = y;

	y = GetHeight() - SaveArgsCheckbox->GetPreferredHeight();
	SaveArgsCheckbox->SetFrameGeometry(0.0, y, GetWidth(), SaveArgsCheckbox->GetPreferredHeight());

	const double editHeight = 34.0;
	y -= editHeight + 8.0;
	ParametersEdit->SetFrameGeometry(0.0, y, GetWidth(), editHeight);

	const double labelHeight = ParametersLabel->GetPreferredHeight();
	y -= labelHeight + 4.0;
	ParametersLabel->SetFrameGeometry(0.0, y, GetWidth(), labelHeight);

	y -= 14.0;
	GamesList->SetFrameGeometry(0.0, listViewTop, GetWidth(), std::max(y - listViewTop, 0.0));

	Launcher->UpdatePlayButton();

	GamesList->ScrollToItem(GamesList->GetSelectedItem());
}
