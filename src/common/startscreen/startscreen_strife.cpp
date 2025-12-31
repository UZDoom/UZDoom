/*
** startscreen_strife.cpp
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
#include "image.h"
#include "textures.h"
#include "palettecontainer.h"
#include "cmdlib.h"

// Strife startup screen
#define PEASANT_INDEX			0
#define LASER_INDEX				4
#define BOT_INDEX				6

#define ST_LASERSPACE_X			60
#define ST_LASERSPACE_Y			156
#define ST_LASERSPACE_WIDTH		200
#define ST_LASER_WIDTH			16
#define ST_LASER_HEIGHT			16

#define ST_BOT_X				14
#define ST_BOT_Y				138
#define ST_BOT_WIDTH			48
#define ST_BOT_HEIGHT			48

#define ST_PEASANT_X			262
#define ST_PEASANT_Y			136
#define ST_PEASANT_WIDTH		32
#define ST_PEASANT_HEIGHT		64

static const char* StrifeStartupPicNames[] =
{
	"STRTPA1", "STRTPB1", "STRTPC1", "STRTPD1",
	"STRTLZ1", "STRTLZ2",
	"STRTBOT",
	"STARTUP0"
};



class FStrifeStartScreen : public FStartScreen
{
public:
	FStrifeStartScreen(int max_progress);

	bool DoProgress(int) override;
protected:
	void DrawStuff(int old_laser, int new_laser);

	FBitmap StartupPics[4+2+1+1];
	int NotchPos = 0;
};


//==========================================================================
//
// FStrifeStartScreen Constructor
//
// Shows the Strife startup screen. If the screen doesn't appear to be
// valid, it returns a failure code in hr.
//
// The startup background is a raw 320x200 image, however Strife only
// actually uses 95 rows from it, starting at row 57. The rest of the image
// is discarded. (What a shame.)
//
// The peasants are raw 32x64 images. The laser dots are raw 16x16 images.
// The bot is a raw 48x48 image. All use the standard PLAYPAL.
//
//==========================================================================

FStrifeStartScreen::FStrifeStartScreen(int max_progress)
	: FStartScreen(max_progress)
{
	StartupBitmap.Create(320, 200);

	// at this point we do not have a working texture manager yet, so we have to do the lookup via the file system
	// Load the background and animated overlays.
	for (size_t i = 0; i < countof(StrifeStartupPicNames); ++i)
	{
		int lumpnum = fileSystem.CheckNumForName(StrifeStartupPicNames[i], FileSys::ns_graphics);
		if (lumpnum < 0) lumpnum = fileSystem.CheckNumForName(StrifeStartupPicNames[i]);

		if (lumpnum >= 0)
		{
			auto lumpr1 = FImageSource::GetImage(lumpnum, false);
			if (lumpr1) StartupPics[i] = lumpr1->GetCachedBitmap(nullptr, FImageSource::normal);
		}
	}
	if (StartupPics[7].GetWidth() != 320 || StartupPics[7].GetHeight() != 200)
	{
		I_Error("bad startscreen assets");
	}

	// Make the startup image appear.
	DrawStuff(0, 0);
	Scale = 2;
	CreateHeader();
}

//==========================================================================
//
// FStrifeStartScreen :: Progress
//
// Bumps the progress meter one notch.
//
//==========================================================================

bool FStrifeStartScreen::DoProgress(int advance)
{
	int notch_pos;

	if (CurPos < MaxPos)
	{
		notch_pos = ((CurPos + 1) * (ST_LASERSPACE_WIDTH - ST_LASER_WIDTH)) / MaxPos;
		if (notch_pos != NotchPos && !(notch_pos & 1))
		{ // Time to update.
			DrawStuff(NotchPos, notch_pos);
			NotchPos = notch_pos;
			StartupTexture->CleanHardwareData(true);
		}
	}
	return FStartScreen::DoProgress(advance);
}

//==========================================================================
//
// FStrifeStartScreen :: DrawStuff
//
// Draws all the moving parts of Strife's startup screen. If you're
// running off a slow drive, it can look kind of good. Otherwise, it
// borders on crazy insane fast.
//
//==========================================================================

void FStrifeStartScreen::DrawStuff(int old_laser, int new_laser)
{
	int y;

	// Clear old laser
	StartupBitmap.Blit(0, 0, StartupPics[7]);

	// Draw new laser
	auto& lp = StartupPics[LASER_INDEX + (new_laser & 1)];
	StartupBitmap.Blit(ST_LASERSPACE_X + new_laser, ST_LASERSPACE_Y, lp);

	// The bot jumps up and down like crazy.
	y = max(0, (new_laser >> 1) % 5 - 2);

	StartupBitmap.Blit(ST_BOT_X, ST_BOT_Y + y, StartupPics[BOT_INDEX]);

	// The peasant desperately runs in place, trying to get away from the laser.
	// Yet, despite all his limb flailing, he never manages to get anywhere.
	auto& pp = StartupPics[PEASANT_INDEX + ((new_laser >> 1) & 3)];
	StartupBitmap.Blit(ST_PEASANT_X, ST_PEASANT_Y, pp);
}


FStartScreen* CreateStrifeStartScreen(int max_progress)
{
	return new FStrifeStartScreen(max_progress);
}
