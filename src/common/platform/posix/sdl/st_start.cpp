/*
** st_start.cpp
**
** Handles the startup screen.
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#include <csignal>
#include <stdio.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "basics.h"
#include "st_start.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class FTTYStartupScreen : public FStartupScreen
{
	public:
		FTTYStartupScreen(int max_progress);
		~FTTYStartupScreen();

		void Progress(int amount = 1) override;

	protected:
		bool DidNetInit;
		int NetMaxPos, NetCurPos;
		const char *TheNetMessage;
		termios OldTermIOS;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern void RedrawProgressBar(int CurPos, int MaxPos);
extern void CleanProgressBar();
extern volatile sig_atomic_t gameloop_abort;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static const char SpinnyProgressChars[4] = { '|', '/', '-', '\\' };

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
	return new FTTYStartupScreen(max_progress);
}

//===========================================================================
//
// FTTYStartupScreen Constructor
//
// Sets the size of the progress bar and displays the startup screen.
//
//===========================================================================

FTTYStartupScreen::FTTYStartupScreen(int max_progress)
	: FStartupScreen(max_progress)
{
	DidNetInit = false;
	NetMaxPos = 0;
	NetCurPos = 0;
	TheNetMessage = NULL;
}

//===========================================================================
//
// FTTYStartupScreen Destructor
//
// Called just before entering graphics mode to deconstruct the startup
// screen.
//
//===========================================================================

FTTYStartupScreen::~FTTYStartupScreen()
{
	//NetDone();	// Just in case it wasn't called yet and needs to be.
}

//===========================================================================
//
// FTTYStartupScreen :: Progress
//
//===========================================================================

void FTTYStartupScreen::Progress(int advance)
{
	CurPos = min(CurPos + advance, MaxPos);
	RedrawProgressBar(CurPos, MaxPos);
}
