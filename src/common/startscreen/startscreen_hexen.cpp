/*
** startscreen_hexen.cpp
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
#include "startupinfo.h"
#include "s_music.h"
#include "image.h"
#include "texturemanager.h"

// Hexen startup screen
#define ST_PROGRESS_X			64			// Start of notches x screen pos.
#define ST_PROGRESS_Y			441			// Start of notches y screen pos.

#define ST_NETPROGRESS_X		288
#define ST_NETPROGRESS_Y		32

class FHexenStartScreen : public FStartScreen
{
	// Hexen's notch graphics, converted to chunky pixels.
	FBitmap Background;
	FBitmap NotchBits;
	FBitmap NetNotchBits;
	int NotchPos = 0;

public:
	FHexenStartScreen(int max_progress);

	bool DoProgress(int) override;
	void DoNetProgress(int count) override;
	void NetDone() override;
};


//==========================================================================
//
// FHexenStartScreen Constructor
//
// Shows the Hexen startup screen. If the screen doesn't appear to be
// valid, it sets hr for a failure.
//
// The startup graphic is a planar, 4-bit 640x480 graphic preceded by a
// 16 entry (48 byte) VGA palette.
//
//==========================================================================

FHexenStartScreen::FHexenStartScreen(int max_progress)
	: FStartScreen(max_progress)
{
	// at this point we do not have a working texture manager yet, so we have to do the lookup via the file system
	int startup_lump = fileSystem.CheckNumForName("STARTUP", FileSys::ns_graphics);
	int netnotch_lump = fileSystem.CheckNumForName("NETNOTCH", FileSys::ns_graphics);
	int notch_lump = fileSystem.CheckNumForName("NOTCH", FileSys::ns_graphics);

	// For backwards compatibility we also need to look in the default namespace, because these were previously not handled as graphics.
	if (startup_lump == -1) startup_lump = fileSystem.CheckNumForName("STARTUP");
	if (netnotch_lump == -1)netnotch_lump = fileSystem.CheckNumForName("NETNOTCH");
	if (notch_lump == -1)notch_lump = fileSystem.CheckNumForName("NOTCH");


	if (startup_lump < 0 || netnotch_lump < 0 || notch_lump < 0)
	{
		I_Error("Start screen assets missing");
	}

	auto iBackground = FImageSource::GetImage(startup_lump, false);
	auto iNetNotchBits = FImageSource::GetImage(netnotch_lump, false);
	auto iNotchBits = FImageSource::GetImage(notch_lump, false);
	if (!iBackground || !iNetNotchBits || !iNotchBits || iBackground->GetWidth() != 640 || iBackground->GetHeight() != 480)
	{
		I_Error("Start screen assets missing");
	}
	NetNotchBits = iNetNotchBits->GetCachedBitmap(nullptr, FImageSource::normal);
	NotchBits = iNotchBits->GetCachedBitmap(nullptr, FImageSource::normal);
	Background = iBackground->GetCachedBitmap(nullptr, FImageSource::normal);

	StartupBitmap.Create(640, 480);
	StartupBitmap.Blit(0, 0, Background, 640, 480);

	// Fill in the bitmap data. Convert to chunky, because I can't figure out
	// if Windows actually supports planar images or not, despite the presence
	// of biPlanes in the BITMAPINFOHEADER.


	if (!batchrun)
	{
		if (GameStartupInfo.Song.IsNotEmpty())
		{
			S_ChangeMusic(GameStartupInfo.Song.GetChars(), true, true);
		}
		else
		{
			S_ChangeMusic("orb", true, true);
		}
	}
	CreateHeader();
}

//==========================================================================
//
// FHexenStartScreen :: Progress
//
// Bumps the progress meter one notch.
//
//==========================================================================

bool FHexenStartScreen::DoProgress(int advance)
{
	int notch_pos, x, y;

	if (CurPos <= MaxPos)
	{
		int numnotches = (16 * 32) / NotchBits.GetWidth();
		notch_pos = ((CurPos + 1) * numnotches) / MaxPos;
		if (notch_pos != NotchPos)
		{ // Time to draw another notch.
			for (; NotchPos < notch_pos; NotchPos++)
			{
				x = ST_PROGRESS_X + NotchBits.GetWidth() * NotchPos;
				y = ST_PROGRESS_Y;
				StartupBitmap.Blit(x, y, NotchBits);
			}
			StartupTexture->CleanHardwareData(true);
			ST_Sound("StartupTick");
		}
	}
	return FStartScreen::DoProgress(advance);
}

//==========================================================================
//
// FHexenStartScreen :: NetProgress
//
// Draws the red net noches in addition to the normal progress bar.
//
//==========================================================================

void FHexenStartScreen::DoNetProgress(int count)
{
	int oldpos = NetCurPos;
	int x, y;

	FStartScreen::NetProgress(count);
	
	if (NetMaxPos != 0 && NetCurPos > oldpos)
	{
		int numnotches = (4 * 8) / NetNotchBits.GetWidth();
		int notch_pos = (NetCurPos * numnotches) / NetMaxPos;

		for (; oldpos < NetCurPos && oldpos < numnotches; ++oldpos)
		{
			x = ST_NETPROGRESS_X + NetNotchBits.GetWidth() * oldpos;
			y = ST_NETPROGRESS_Y;
			StartupBitmap.Blit(x, y, NetNotchBits);
		}
		ST_Sound("misc/netnotch");
		StartupTexture->CleanHardwareData(true);
	}
}

//==========================================================================
//
// FHexenStartScreen :: NetDone
//
// Aside from the standard processing, also plays a sound.
//
//==========================================================================

void FHexenStartScreen::NetDone()
{
	ST_Sound("PickupWeapon");
	FStartScreen::NetDone();
}


FStartScreen* CreateHexenStartScreen(int max_progress)
{
	return new FHexenStartScreen(max_progress);
}
