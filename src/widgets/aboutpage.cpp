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

#include "aboutpage.h"

#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>
#include <zwidget/core/widget.h>
#include <cmath>
#include <cstring>
#include <functional>
#include <string>
#include <sstream>

#include "filesystem.h"
#include "findfile.h"
#include "fs_files.h"
#include "gameconfigfile.h"
#include "gstrings.h"
#include "i_interface.h"
#include "launcherwindow.h"
#include "releasepage.h"
#include "resourcefile.h"
#include "stringtable.h"
#include "version.h"
#include "zstring.h"
#include "textblock.h"

#ifdef HAS_UPDATER
#include "curl_loader.h"
#endif

AboutPage::AboutPage(LauncherWindow* launcher, const FStartupSelectionInfo& info) : Widget(nullptr), Launcher(launcher)
{
	Text = new TextBlock(this);
	Notes = new PushButton(this);

	auto wad = BaseFileSearch(BASEWAD, NULL, true, GameConfig);
	if (wad)
	{
		// we need to be free
		auto resf = FResourceFile::OpenResourceFile(wad);
		FString text;

		auto getText = [&resf](const char * name)->std::string {
			auto lump = resf->FindEntry(name);
			if (lump < 0) return "";
			auto data = resf->Read(lump);
			return {data.string(), data.size()};
		};
		auto replace = [](std::string &s, const std::string &a, const std::string &b)
		{
			size_t pos = s.find(a);
			while (pos != std::string::npos)
			{
				s.replace(pos, a.size(), b);
				pos = s.find(a, pos + b.size());
			}
		};

		int lump;
		if (resf)
		{
			{
				auto str = getText("about.txt");
				text.AppendCStrPart(str.c_str(), str.size());
			}
			text.AppendCharacter('\n');

			auto ss = std::stringstream{getText("contributors.txt")};
			size_t count = 0;
			for (std::string line; std::getline(ss, line, '\n'); count++)
			{
				const char* fmt = count==0? "%s": "\u00a0· %s";
				replace(line, " ", "\u00a0");
				text.AppendFormat(fmt, line.c_str());
			}

			text.StripLeftRight();
		}

		delete resf;

		Text->SetText(text.GetChars());
	}

	Notes->SetText(GStrings.GetString("PICKER_SHOWNOTES"));

	Notes->OnClick = [=,this]()
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

#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		ForceUpdate = new PushButton(this);
		ForceUpdate->SetText(GStrings.GetString("UPDATER_CHECK_FOR_UPDATES"));

		ForceUpdate->OnClick = [=,this]()
		{
			static_cast<LauncherWindow*>(Window())->ForceCheckUpdate();
		};
	}
#endif
}

void AboutPage::SetValues(FStartupSelectionInfo& info) const
{
}

void AboutPage::UpdateLanguage()
{
	Notes->SetText(GStrings.GetString("PICKER_SHOWNOTES"));
}

void AboutPage::OnGeometryChanged()
{
	double y = 0.0;
	double w = GetWidth();
	double h = GetHeight();
	double tw = 0, th, tx;

	th = Notes->GetPreferredHeight();
	Text->SetFrameGeometry(0.0, y, w, h - th - 8.0);
	y += h - th;

	tw += Notes->GetPreferredWidth();
#ifdef HAS_UPDATER
	if(IsCurlLoaded()) tw += ForceUpdate->GetPreferredWidth() + 8;
#endif
	tx = round((w-tw)/2);
	tw = Notes->GetPreferredWidth();
	Notes->SetFrameGeometry(tx, y, tw, th);
	tx += tw + 8;
#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		tw = ForceUpdate->GetPreferredWidth();
		ForceUpdate->SetFrameGeometry(tx, y, tw, th);
	}
#endif
	y += h;

	Launcher->UpdatePlayButton();
}
