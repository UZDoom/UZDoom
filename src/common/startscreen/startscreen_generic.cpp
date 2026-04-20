/*
** startscreen_generic.cpp
**
** Generic startup screen
**
**---------------------------------------------------------------------------
**
** Copyright 2022 Christoph Oelckers
** Copyright 2022-2025 GZDoom Maintainers and Contributors
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
#include "startupinfo.h"
#include "image.h"
#include "texturemanager.h"
#include "widgets/themedata.h"

// Hexen startup screen
#define ST_PROGRESS_X			64			// Start of notches x screen pos.
#define ST_PROGRESS_Y			441			// Start of notches y screen pos.


class FGenericStartScreen : public FStartScreen
{
	FBitmap Background;
	int NotchPos = 0;

public:
	FGenericStartScreen(int max_progress);

	bool DoProgress(int) override;
};


//==========================================================================
//
// FGenericStartScreen Constructor
//
// Shows the Hexen startup screen. If the screen doesn't appear to be
// valid, it sets hr for a failure.
//
// The startup graphic is a planar, 4-bit 640x480 graphic preceded by a
// 16 entry (48 byte) VGA palette.
//
//==========================================================================

FGenericStartScreen::FGenericStartScreen(int max_progress)
	: FStartScreen(max_progress)
{
	// at this point we do not have a working texture manager yet, so we have to do the lookup via the file system
	int startup_lump = fileSystem.CheckNumForName("BOOTLOGO", FileSys::ns_graphics);

	StartupBitmap.Create(640 * 2, 480 * 2);
	ClearBlock(StartupBitmap, { 0, 0, 0, 255 }, 0, 0, 640 * 2, 480 * 2);
	// This also needs to work if the lump turns out to be unusable.
	if (startup_lump != -1)
	{
		auto iBackground = FImageSource::GetImage(startup_lump, false);
		if (iBackground)
		{
			Background = iBackground->GetCachedBitmap(nullptr, FImageSource::normal);
			if (Background.GetWidth() < 640 * 2 || Background.GetHeight() < 480 * 2)
				StartupBitmap.Blit(320 * 2 - Background.GetWidth()/2, 220 * 2 - Background.GetHeight() / 2, Background);
			else
				StartupBitmap.Blit(0, 0, Background, 640 * 2, 480 * 2);
		
		}
	}
}

//==========================================================================
//
// FGenericStartScreen :: Progress
//
// Bumps the progress meter one notch.
//
//==========================================================================

bool FGenericStartScreen::DoProgress(int advance)
{
	static auto argb = 0;
	static RgbQuad bcolor = { 255, 255, 255, 255 };
	if (!argb && bcolor.rgbReserved)
	{
		argb = Theme::getAccent().toBgra8();
		bcolor.rgbRed      = static_cast<unsigned char>(0xff&(argb>>16));
		bcolor.rgbGreen    = static_cast<unsigned char>(0xff&(argb>>8));
		bcolor.rgbBlue     = static_cast<unsigned char>(0xff&(argb>>0));
		bcolor.rgbReserved = static_cast<unsigned char>(0xff&(argb>>24));
	}

	if (CurPos < MaxPos)
	{
		int numnotches = 200 * 2;
		int notch_pos = ((CurPos + 1) * numnotches) / MaxPos;
		if (notch_pos != NotchPos)
		{ // Time to draw another notch.
			ClearBlock(StartupBitmap, bcolor, (320 - 100) * 2, 480 * 2 - 30, notch_pos, 4 * 2);
			NotchPos = notch_pos;
			if (StartupTexture)
				StartupTexture->CleanHardwareData(true);
		}
	}
	return FStartScreen::DoProgress(advance);
}


FStartScreen* CreateGenericStartScreen(int max_progress)
{
	return new FGenericStartScreen(max_progress);
}
