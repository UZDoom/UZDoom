/*
** st_start.mm
**
**---------------------------------------------------------------------------
**
** Copyright 2015-2018 Alexey Lysiuk
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include <unistd.h>

#import <Foundation/NSRunLoop.h>

#include "c_cvars.h"
#include "st_console.h"
#include "st_start.h"
#include "printf.h"
#include "engineerrors.h"


// ---------------------------------------------------------------------------


FBasicStartupScreen::FBasicStartupScreen(int maxProgress)
: FStartupScreen(maxProgress)
{
	FConsoleWindow& consoleWindow = FConsoleWindow::GetInstance();
	consoleWindow.SetProgressBar(true);

#if 0
	// Testing code, please do not remove
	consoleWindow.AddText("----------------------------------------------------------------\n");
	consoleWindow.AddText("1234567890 !@#$%^&*() ,<.>/?;:'\" [{]}\\| `~-_=+ "
		"This is very very very long message needed to trigger word wrapping...\n\n");
	consoleWindow.AddText("Multiline...\n\tmessage...\n\t\twith...\n\t\t\ttabs.\n\n");

	consoleWindow.AddText(TEXTCOLOR_BRICK "TEXTCOLOR_BRICK\n" TEXTCOLOR_TAN "TEXTCOLOR_TAN\n");
	consoleWindow.AddText(TEXTCOLOR_GRAY "TEXTCOLOR_GRAY & TEXTCOLOR_GREY\n");
	consoleWindow.AddText(TEXTCOLOR_GREEN "TEXTCOLOR_GREEN\n" TEXTCOLOR_BROWN "TEXTCOLOR_BROWN\n");
	consoleWindow.AddText(TEXTCOLOR_GOLD "TEXTCOLOR_GOLD\n" TEXTCOLOR_RED "TEXTCOLOR_RED\n");
	consoleWindow.AddText(TEXTCOLOR_BLUE "TEXTCOLOR_BLUE\n" TEXTCOLOR_ORANGE "TEXTCOLOR_ORANGE\n");
	consoleWindow.AddText(TEXTCOLOR_WHITE "TEXTCOLOR_WHITE\n" TEXTCOLOR_YELLOW "TEXTCOLOR_YELLOW\n");
	consoleWindow.AddText(TEXTCOLOR_UNTRANSLATED "TEXTCOLOR_UNTRANSLATED\n");
	consoleWindow.AddText(TEXTCOLOR_BLACK "TEXTCOLOR_BLACK\n" TEXTCOLOR_LIGHTBLUE "TEXTCOLOR_LIGHTBLUE\n");
	consoleWindow.AddText(TEXTCOLOR_CREAM "TEXTCOLOR_CREAM\n" TEXTCOLOR_OLIVE "TEXTCOLOR_OLIVE\n");
	consoleWindow.AddText(TEXTCOLOR_DARKGREEN "TEXTCOLOR_DARKGREEN\n" TEXTCOLOR_DARKRED "TEXTCOLOR_DARKRED\n");
	consoleWindow.AddText(TEXTCOLOR_DARKBROWN "TEXTCOLOR_DARKBROWN\n" TEXTCOLOR_PURPLE "TEXTCOLOR_PURPLE\n");
	consoleWindow.AddText(TEXTCOLOR_DARKGRAY "TEXTCOLOR_DARKGRAY\n" TEXTCOLOR_CYAN "TEXTCOLOR_CYAN\n");
	consoleWindow.AddText(TEXTCOLOR_ICE "TEXTCOLOR_ICE\n" TEXTCOLOR_FIRE "TEXTCOLOR_FIRE\n");
	consoleWindow.AddText(TEXTCOLOR_SAPPHIRE "TEXTCOLOR_SAPPHIRE\n" TEXTCOLOR_TEAL "TEXTCOLOR_TEAL\n");
	consoleWindow.AddText(TEXTCOLOR_NORMAL "TEXTCOLOR_NORMAL\n" TEXTCOLOR_BOLD "TEXTCOLOR_BOLD\n");
	consoleWindow.AddText(TEXTCOLOR_CHAT "TEXTCOLOR_CHAT\n" TEXTCOLOR_TEAMCHAT "TEXTCOLOR_TEAMCHAT\n");
	consoleWindow.AddText("----------------------------------------------------------------\n");
#endif // _DEBUG
}

FBasicStartupScreen::~FBasicStartupScreen()
{
	FConsoleWindow::GetInstance().SetProgressBar(false);
}


void FBasicStartupScreen::Progress(int advance)
{
	CurPos = min(CurPos + advance, MaxPos);
	FConsoleWindow::GetInstance().Progress(CurPos, MaxPos);
}


void FBasicStartupScreen::NetInit(const char* const message, const bool host)
{
	FConsoleWindow::GetInstance().NetInit(message, host);
}

void FBasicStartupScreen::NetMessage(const char* const message)
{
	FConsoleWindow::GetInstance().NetMessage(message);
}

void FBasicStartupScreen::NetConnect(const int client, const char* const name, const unsigned flags, const int status)
{
	FConsoleWindow::GetInstance().NetConnect(client, name, flags, status);
}

void FBasicStartupScreen::NetUpdate(const int client, const int status)
{
	FConsoleWindow::GetInstance().NetUpdate(client, status);
}

void FBasicStartupScreen::NetDisconnect(const int client)
{
	FConsoleWindow::GetInstance().NetDisconnect(client);
}

void FBasicStartupScreen::NetProgress(const int cur, const int limit)
{
	FConsoleWindow::GetInstance().NetProgress(cur, limit);
}

void FBasicStartupScreen::NetDone()
{
	FConsoleWindow::GetInstance().NetDone();
}

void FBasicStartupScreen::NetClose()
{
	FConsoleWindow::GetInstance().NetClose();
}

bool FBasicStartupScreen::ShouldStartNet()
{
	return FConsoleWindow::GetInstance().ShouldStartNet();
}

int FBasicStartupScreen::GetNetKickClient()
{
	return FConsoleWindow::GetInstance().GetNetKickClient();
}

int FBasicStartupScreen::GetNetBanClient()
{
	return FConsoleWindow::GetInstance().GetNetBanClient();
}

bool FBasicStartupScreen::NetLoop(bool (*loopCallback)(void*), void* const data)
{
	while (true)
	{
		if (loopCallback(data))
		{
			break;
		}

		[[NSRunLoop currentRunLoop] limitDateForMode:NSDefaultRunLoopMode];

		// Do not poll to often
		usleep(50000);
	}

	return true;
}


// ---------------------------------------------------------------------------


FStartupScreen *FStartupScreen::CreateInstance(const int maxProgress)
{
	return new FBasicStartupScreen(maxProgress);
}
