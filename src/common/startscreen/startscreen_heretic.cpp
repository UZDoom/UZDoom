/*
** startscreen_heretic.cpp
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

#include "startscreen.h"
#include "filesystem.h"
#include "printf.h"
#include "texturemanager.h"


// Heretic startup screen
#define HERETIC_MINOR_VERSION	'3'			// Since we're based on Heretic 1.3
#define THERM_X					14
#define THERM_Y					14
#define THERM_LEN				51
#define THERM_COLOR				0xA		// light green


class FHereticStartScreen : public FStartScreen
{
	int NotchPos;
	int ThermX, ThermY, ThermWidth, ThermHeight;
	int HMsgY, SMsgX;
public:
	FHereticStartScreen(int max_progress);

	bool DoProgress(int) override;
	void LoadingStatus(const char *message, int colors) override;
	void AppendStatusLine(const char *status) override;
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

FHereticStartScreen::FHereticStartScreen(int max_progress)
	: FStartScreen(max_progress)
{
	int loading_lump = fileSystem.CheckNumForName("LOADING");
	uint8_t loading_screen[4000];

	if (loading_lump < 0 || fileSystem.FileLength(loading_lump) != 4000)
	{
		I_Error("'LOADING' not found");
	}

	fileSystem.ReadFile(loading_lump, loading_screen);

	// Slap the Heretic minor version on the loading screen. Heretic
	// did this inside the executable rather than coming with modified
	// LOADING screens, so we need to do the same.
	loading_screen[2 * 160 + 49 * 2] = HERETIC_MINOR_VERSION;

	// Draw the loading screen to a bitmap.
	StartupBitmap.Create(80 * 8, 25 * 16);
	DrawTextScreen(StartupBitmap, loading_screen);

	ThermX = THERM_X * 8;
	ThermY = THERM_Y * 16;
	ThermWidth = THERM_LEN * 8 - 4;
	ThermHeight = 16;
	HMsgY = 7;
	SMsgX = 1;
	NotchPos = 0;
	CreateHeader();
}

//==========================================================================
//
// FHereticStartScreen::Progress
//
// Bumps the progress meter one notch.
//
//==========================================================================

bool FHereticStartScreen::DoProgress(int advance)
{
	if (CurPos < MaxPos)
	{
		int notch_pos = ((CurPos + 1) * ThermWidth) / MaxPos;
		if (notch_pos != NotchPos && !(notch_pos & 3))
		{ // Time to draw another notch.
			int left = NotchPos + ThermX;
			int top = ThermY;
			int right = notch_pos + ThermX;
			int bottom = top + ThermHeight;
			ClearBlock(StartupBitmap, TextModePalette[THERM_COLOR], left, top, right - left, bottom - top);
			NotchPos = notch_pos;
			StartupTexture->CleanHardwareData(true);
		}
	}
	return FStartScreen::DoProgress(advance);
}

//==========================================================================
//
// FHereticStartScreen :: LoadingStatus
//
// Prints text in the center box of the startup screen.
//
//==========================================================================

void FHereticStartScreen::LoadingStatus(const char* message, int colors)
{
	int x;

	for (x = 0; message[x] != '\0'; ++x)
	{
		DrawChar(StartupBitmap, 17 + x, HMsgY, message[x], colors);
	}
	HMsgY++;
	StartupTexture->CleanHardwareData(true);
	Render();
}

//==========================================================================
//
// FHereticStartScreen :: AppendStatusLine
//
// Appends text to Heretic's status line.
//
//==========================================================================

void FHereticStartScreen::AppendStatusLine(const char* status)
{
	int x;

	for (x = 0; status[x] != '\0'; ++x)
	{
		DrawChar(StartupBitmap, SMsgX + x, 24, status[x], 0x1f);
	}
	SMsgX += x;
	StartupTexture->CleanHardwareData(true);
	Render();
}


FStartScreen* CreateHereticStartScreen(int max_progress)
{
	return new FHereticStartScreen(max_progress);
}
