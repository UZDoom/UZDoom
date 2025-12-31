/*
** endoom.cpp
**
** Handles the startup screen.
**
**---------------------------------------------------------------------------
**
** Copyright 2006-2016 Marisa Heit
** Copyright 2006-2022 Christoph Oelckers
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

#include "startscreen.h"
#include "cmdlib.h"

#include "i_system.h"
#include "gstrings.h"
#include "filesystem.h"
#include "m_argv.h"
#include "engineerrors.h"
#include "s_music.h"
#include "printf.h"
#include "startupinfo.h"
#include "i_interface.h"
#include "texturemanager.h"
#include "c_cvars.h"
#include "i_time.h"
#include "g_input.h"
#include "d_eventbase.h"

// MACROS ------------------------------------------------------------------

// How many ms elapse between blinking text flips. On a standard VGA
// adapter, the characters are on for 16 frames and then off for another 16.
// The number here therefore corresponds roughly to the blink rate on a
// 60 Hz display.
#define BLINK_PERIOD			267


// TYPES -------------------------------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

CUSTOM_CVAR(Int, showendoom, 0, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
{
	if (self < 0) self = 0;
	else if (self > 2) self=2;
}

CVAR(Bool, consoleendoom, true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)

#ifdef _WIN32
extern bool FancyStdOut;
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

class FEndoomScreen : public FStartScreen
{
	uint64_t lastUpdateTime;
	bool blinkstate = false;
	bool blinking = true;
	uint8_t endoom_screen[4000];

public:
	FEndoomScreen(int);
	void Update();
};


//==========================================================================
//
// FHereticStartScreen Constructor
//
// Shows the Heretic startup screen. If the screen doesn't appear to be
// valid, it returns a failure code in hr.
//
// The loading screen is an 80x25 text screen with character data and
// attributes intermixed, which means it must be exactly 4000 bytes long.
//
//==========================================================================

FEndoomScreen::FEndoomScreen(int loading_lump)
	: FStartScreen(0)
{
	fileSystem.ReadFile(loading_lump, endoom_screen);

	// Draw the loading screen to a bitmap.
	StartupBitmap.Create(80 * 8, 26 * 16); // line 26 is for our own 'press any key to quit' message.
	DrawTextScreen(StartupBitmap, endoom_screen);
	ClearBlock(StartupBitmap, {0, 0, 0, 255}, 0, 25*16, 640, 16);
	DrawString(StartupBitmap, 0, 25, GStrings.GetString("TXT_QUITENDOOM"), { 128, 128, 128 ,255}, { 0, 0, 0, 255});
	lastUpdateTime = I_msTime();
	
	// Does this screen need blinking?
	for (int i = 0; i < 80*25; ++i)
	{
		if (endoom_screen[1+i*2] & 0x80)
		{
			blinking = true;
			break;
		}
	}
}

void FEndoomScreen::Update()
{
	if (blinking && I_msTime() > lastUpdateTime + BLINK_PERIOD)
	{
		lastUpdateTime = I_msTime();
		UpdateTextBlink (StartupBitmap, endoom_screen, blinkstate);
		blinkstate = !blinkstate;
		StartupTexture->CleanHardwareData();
		Render(true);
	}
}


//==========================================================================
//
// ST_Endoom
//
// Shows an ENDOOM text screen
//
//==========================================================================

int RunEndoom()
{
	if (showendoom == 0 || endoomName.Len() == 0)
	{
		return 0;
	}

	int endoom_lump = fileSystem.CheckNumForFullName (endoomName.GetChars(), true);
	
	if (endoom_lump < 0 || fileSystem.FileLength (endoom_lump) != 4000)
	{
		return 0;
	}

	if (fileSystem.GetFileContainer(endoom_lump) == fileSystem.GetMaxIwadNum() && showendoom == 2)
	{
		// showendoom==2 means to show only lumps from PWADs.
		return 0;
	}
	
	S_StopMusic(true);
	auto endoom = new FEndoomScreen(endoom_lump);
	endoom->Render(true);

	while(true)
	{
		I_GetEvent();
		endoom->Update();
		while (eventtail != eventhead)
		{
			event_t *ev = &events[eventtail];
			eventtail = (eventtail + 1) & (MAXEVENTS - 1);

			if (ev->type == EV_KeyDown || ev->type == EV_KeyUp)
			{
				return 0;
			}
			if (ev->type == EV_GUI_Event && (ev->subtype == EV_GUI_KeyDown || ev->subtype == EV_GUI_LButtonDown || ev->subtype == EV_GUI_RButtonDown || ev->subtype == EV_GUI_MButtonDown))
			{
				return 0;
			}
		}
	}
	return 0;
}

void vga_to_ansi(const uint8_t *buf);

void ConsoleEndoom()
{
#ifdef _WIN32
	// old versions of Windows don't get an ansi endoom
	if (!FancyStdOut)
		return;
#endif

	if (!consoleendoom || endoomName.Len() == 0)
		return;

	uint8_t buffer[4000];

	int endoom_lump = fileSystem.CheckNumForFullName (endoomName.GetChars(), true);
	
	if (endoom_lump < 0 || fileSystem.FileLength (endoom_lump) != 4000)
	{
		return;
	}
	fileSystem.ReadFile(endoom_lump, buffer);

	vga_to_ansi(buffer);
}

[[noreturn]]
void ST_Endoom()
{
	ConsoleEndoom();
	int code = RunEndoom();
	throw CExitEvent(code);
}
