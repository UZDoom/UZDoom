/*
** aboutpage.cpp
**
** About tab of launcher
**
**---------------------------------------------------------------------------
**
** Copyright 2025 Marcus Minhorst
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include <cstring>

#include <zwidget/widgets/checkboxlabel/checkboxlabel.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/textedit/textedit.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>

#include "aboutpage.h"
#include "filesystem.h"
#include "findfile.h"
#include "gameconfigfile.h"
#include "gstrings.h"
#include "i_interface.h"
#include "launcherwindow.h"
#include "name.h"
#include "releasepage.h"
#include "version.h"
#include "zstring.h"

AboutPage::AboutPage(LauncherWindow* launcher, const FStartupSelectionInfo& info) : Widget(nullptr), Launcher(launcher)
{
	// [Marcus] TODO: Probably make this rich-text
	Text = new TextEdit(this);
	Notes = new PushButton(this);

	auto wad = BaseFileSearch(BASEWAD, NULL, true, GameConfig);
	if (wad)
	{
		// we need to be free
		auto resf = FResourceFile::OpenResourceFile(wad);
		FString text;

		auto append = [&resf,&text](const char * name) {
			auto lump = resf->FindEntry(name);
			if (lump < 0) return;
			auto data = resf->Read(lump);
			text.AppendCStrPart(data.string(), data.size());
		};

		int lump;
		if (resf)
		{
			append("about.txt");

			// [Marcus] I would love to instead have this done at compile time and also
			// separate the entries by ' · ', but there's currently a bug in zwidget
			// that breaks how long a soft-wrapped line of text can be :(
			text.AppendCharacter('\n');
			append("contributors.txt");

			text.StripLeftRight();
		}

		delete resf;

		Text->SetText(text.GetChars());
	}

	Text->SetReadOnly(true);
	Notes->SetText(GStrings.GetString("PICKER_SHOWNOTES"));

	Notes->OnClick = [=]()
	{
		if (!Launcher->Release)
		{
			Launcher->Release = new ReleasePage(launcher, info);
			Launcher->Pages->AddTab(Launcher->Release, "Release Notes");
			Launcher->UpdateLanguage();
		}

		Launcher->Pages->SetCurrentIndex(Launcher->Pages->GetPageIndex(Launcher->Release));
		Launcher->Pages->GetCurrentWidget()->SetFocus();
	};
}

void AboutPage::SetValues(FStartupSelectionInfo& info) const
{
	Notes->SetText(GStrings.GetString("PICKER_SHOWNOTES"));
}

void AboutPage::UpdateLanguage()
{
}

void AboutPage::OnGeometryChanged()
{
	double y = 0.0;
	double w = GetWidth();
	double h = GetHeight();

	Text->SetFrameGeometry(0.0, y, w, h - Notes->GetPreferredHeight() - 8.0);
	y += h - Notes->GetPreferredHeight();

	// [Marcus] TODO: add PushButton.GetPreferredWidth()
	Notes->SetFrameGeometry(round(w/2) - 100.0, y, 200.0, Notes->GetPreferredHeight());
	y += h;

	Launcher->UpdatePlayButton();
}
