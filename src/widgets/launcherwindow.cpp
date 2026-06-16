/*
** launcherwindow.cpp
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

#include <zwidget/core/resourcedata.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>
#include <zwidget/window/window.h>

#include "aboutpage.h"
#include "gstrings.h"
#include "i_interface.h"
#include "launcherbanner.h"
#include "launcherbuttonbar.h"
#include "launcherwindow.h"
#include "networkpage.h"
#include "playgamepage.h"
#include "releasepage.h"
#include "settingspage.h"
#include "version.h"
#include "themedata.h"

#ifdef HAS_UPDATER
#include "updatebuttonbar.h"
#include "curl_loader.h"
#endif

bool LauncherWindow::ExecModal(FStartupSelectionInfo& info)
{
	auto launcher = std::make_unique<LauncherWindow>(info);

	launcher->UpdateSize();
	launcher->Show();

	DisplayWindow::RunLoop();

	return launcher->ExecResult;
}

void LauncherWindow::UpdateSize()
{
	Size screenSize = GetScreenSize();

	double windowWidth = 615.0;
	double windowHeight = 700.0;

#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		windowHeight += UpdateBar->GetPreferredHeight() + UpdateBar->GetMargin();
	}
#endif

	SetFrameGeometry((screenSize.width - windowWidth) * 0.5, (screenSize.height - windowHeight) * 0.5, windowWidth, windowHeight);
}

LauncherWindow::LauncherWindow(FStartupSelectionInfo& info) : Widget(nullptr, WidgetType::Window, RenderAPI::Unspecified, false), Info(&info)
{
#ifdef HAS_UPDATER
	LoadCurl();
#endif
	SetWindowTitle(GAMENAME);
	this->SetStyleColor("background-color", Theme::getHeader(COLOR_BACKGROUND));

	Banner = new LauncherBanner(this, info.prideColors, info.prideMix);
	Pages = new TabWidget(this);
	Pages->SetStyleColor("background-color", Theme::getMain(COLOR_BACKGROUND));
	Buttonbar = new LauncherButtonbar(this);
	Buttonbar->SetStyleColor("background-color", Theme::getMain(COLOR_BACKGROUND));
#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		UpdateBar = new UpdateButtonBar(this);
		UpdateBar->Hide();
	}
#endif

	bool releasenotes = info.isNewRelease && info.notifyNewRelease;

	PlayGame = new PlayGamePage(this, info);
	Settings = new SettingsPage(this, info);
	Network = new NetworkPage(this, info);
	About = new AboutPage(this, info);

	if (releasenotes)
	{
		Release = new ReleasePage(this, info);
		Pages->AddTab(Release, "Release Notes");
	}

	Pages->AddTab(PlayGame, "Play");
	Pages->AddTab(Settings, "Settings");
	Pages->AddTab(Network, "Multiplayer");
	Pages->AddTab(About, "About");

	Network->InitializeTabs(info);

	UpdateLanguage();

	Pages->SetCurrentIndex(0);
	Pages->GetCurrentWidget()->SetFocus();

#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		UpdateBar->CheckForUpdate();
	}
#endif
}

void LauncherWindow::ForceCheckUpdate()
{
#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		UpdateBar->CheckForUpdate(true);
	}
#endif
}

#ifndef HAS_UPDATER
void LauncherWindow::OnWindowClose()
{
	Close();
}

#endif
void LauncherWindow::UpdatePlayButton()
{
	Buttonbar->UpdateLanguage();
}

bool LauncherWindow::IsInMultiplayer() const
{
	return Pages->GetCurrentIndex() >= 0 ? Pages->GetCurrentWidget() == Network : false;
}

bool LauncherWindow::IsHosting() const
{
	return IsInMultiplayer() && Network->IsInHost();
}

void LauncherWindow::Start()
{
	Info->bNetStart = IsInMultiplayer();

	Settings->SetValues(*Info);
	if (Info->bNetStart)
		Network->SetValues(*Info);
	else
		PlayGame->SetValues(*Info);

	if (Release)
		Release->SetValues(*Info);

	ExecResult = true;
	DisplayWindow::ExitLoop();
}

void LauncherWindow::Exit()
{
	ExecResult = false;
	DisplayWindow::ExitLoop();
}

void LauncherWindow::UpdateLanguage()
{
	Pages->SetTabText(PlayGame, GStrings.GetString("PICKER_TAB_PLAY"));
	Pages->SetTabText(Settings, GStrings.GetString("OPTMNU_TITLE"));
	Pages->SetTabText(Network, GStrings.GetString("PICKER_TAB_MULTI"));
	Pages->SetTabText(About, GStrings.GetString("PICKER_TAB_ABOUT"));
	PlayGame->UpdateLanguage();
	Settings->UpdateLanguage();
	Network->UpdateLanguage();
	About->UpdateLanguage();
	if (Release)
	{
		Pages->SetTabText(Release, GStrings.GetString("PICKER_TAB_RELEASE"));
		Release->UpdateLanguage();
	}
	Buttonbar->UpdateLanguage();

#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		UpdateBar->UpdateLanguage();
	}
#endif
	OnGeometryChanged();
}

void LauncherWindow::OnClose()
{
	Exit();
}

void LauncherWindow::OnGeometryChanged()
{
	double top = 0.0;
	double bottom = GetHeight();

	Banner->SetFrameGeometry(0.0, top, GetWidth(), Banner->GetPreferredHeight());
	top += Banner->GetPreferredHeight();

#ifdef HAS_UPDATER
	if(IsCurlLoaded())
	{
		double updateBarHeight = UpdateBar->GetPreferredHeight();
		UpdateBar->SetFrameGeometry(0.0, top, GetWidth(), updateBarHeight);
		top += updateBarHeight + UpdateBar->GetMargin();
	}
#endif

	bottom -= Buttonbar->GetPreferredHeight();
	Buttonbar->SetFrameGeometry(0.0, bottom, GetWidth(), Buttonbar->GetPreferredHeight());

	Pages->SetFrameGeometry(0.0, top, GetWidth(), std::max(bottom - top, 0.0));
}
