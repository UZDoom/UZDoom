/*
** st_start.cpp
**
** Handles the startup screen.
**
**---------------------------------------------------------------------------
**
** Copyright 2006-2016 Marisa Heit
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

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#include "st_start.h"
#include "cmdlib.h"

#include "i_system.h"
#include "i_input.h"
#include "hardware.h"
#include "filesystem.h"
#include "m_argv.h"
#include "engineerrors.h"
#include "s_music.h"
#include "printf.h"
#include "startupinfo.h"
#include "i_interface.h"
#include "texturemanager.h"
#include "i_mainwindow.h"

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void ST_Util_SizeWindowForBitmap (int scale);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern HINSTANCE g_hInst;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// FStartupScreen :: CreateInstance
//
// Initializes the startup screen for the detected game.
// Sets the size of the progress bar and displays the startup screen.
//
//==========================================================================

FStartupScreen *FStartupScreen::CreateInstance(int max_progress)
{
	return new FBasicStartupScreen(max_progress);
}

//==========================================================================
//
// FBasicStartupScreen Constructor
//
// Shows a progress bar at the bottom of the window.
//
//==========================================================================

FBasicStartupScreen::FBasicStartupScreen(int max_progress)
: FStartupScreen(max_progress)
{
	NetMaxPos = 0;
	NetCurPos = 0;
}

//==========================================================================
//
// FBasicStartupScreen Destructor
//
// Called just before entering graphics mode to deconstruct the startup
// screen.
//
//==========================================================================

FBasicStartupScreen::~FBasicStartupScreen()
{
}

//==========================================================================
//
// FBasicStartupScreen :: Progress
//
// Bumps the progress meter one notch.
//
//==========================================================================

void FBasicStartupScreen::Progress(int advance)
{
	CurPos = min(CurPos + advance, MaxPos);
}

//==========================================================================
//
// FBasicStartupScreen :: NetInit
//
// Shows the network startup pane if it isn't visible. Sets the message in
// the pane to the one provided. If numplayers is 0, then the progress bar
// is a scrolling marquee style. If numplayers is 1, then the progress bar
// is just a full bar. If numplayers is >= 2, then the progress bar is a
// normal one, and a progress count is also shown in the pane.
//
//==========================================================================

void FBasicStartupScreen::NetInit(const char *message, bool host)
{
	mainwindow.NetInit(message, host);
	NetCurPos = 0;
}

void FBasicStartupScreen::NetMessage(const char* message)
{
	mainwindow.NetMessage(message);
}

void FBasicStartupScreen::NetConnect(int client, const char* name, unsigned flags, int status)
{
	mainwindow.NetConnect(client, name, flags, status);
}

void FBasicStartupScreen::NetUpdate(int client, int status)
{
	mainwindow.NetUpdate(client, status);
}

void FBasicStartupScreen::NetDisconnect(int client)
{
	mainwindow.NetDisconnect(client);
}

//==========================================================================
//
// FBasicStartupScreen :: NetProgress
//
// Sets the network progress meter. If count is 0, it gets bumped by 1.
// Otherwise, it is set to count.
//
//==========================================================================

void FBasicStartupScreen::NetProgress(int cur, int limit)
{
	NetMaxPos = limit;
	NetCurPos = cur;
	mainwindow.NetProgress(cur, limit);
}

//==========================================================================
//
// FBasicStartupScreen :: NetDone
//
// Removes the network startup pane.
//
//==========================================================================

void FBasicStartupScreen::NetDone()
{
	mainwindow.NetDone();
}

void FBasicStartupScreen::NetClose()
{
	mainwindow.NetClose();
}

bool FBasicStartupScreen::ShouldStartNet()
{
	return mainwindow.ShouldStartNet();
}

int FBasicStartupScreen::GetNetKickClient()
{
	return mainwindow.GetNetKickClient();
}

int FBasicStartupScreen::GetNetBanClient()
{
	return mainwindow.GetNetBanClient();
}

//==========================================================================
//
// FBasicStartupScreen :: NetLoop
//
// The timer_callback function is called at least two times per second
// and passed the userdata value. It should return true to stop the loop and
// return control to the caller or false to continue the loop.
//
// ST_NetLoop will return true if the loop was halted by the callback and
// false if the loop was halted because the user wants to abort the
// network synchronization.
//
//==========================================================================

bool FBasicStartupScreen::NetLoop(bool (*loopCallback)(void*), void* data)
{
	return mainwindow.NetLoop(loopCallback, data);
}
