/*
** sbarinfo.h
**
** Header for custom status bar definitions.
**
**---------------------------------------------------------------------------
**
** Copyright 2008 Braden Obrzut
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

#ifndef __SBarInfo_SBAR_H__
#define __SBarInfo_SBAR_H__

#include "tarray.h"

#define NUMHUDS 9
#define NUMPOPUPS 3

class FScanner;
class DSBarInfo;
class SBarInfoMainBlock;

//Popups!
struct Popup
{
	enum PopupTransition
	{
		TRANSITION_NONE,
		TRANSITION_SLIDEINBOTTOM,
		TRANSITION_PUSHUP,
		TRANSITION_FADE,
	};

	PopupTransition transition;
	bool opened;
	bool moving;
	int height;
	int width;
	int ispeed;
	double speed;
	double speed2;
	double alpha;
	int x;
	int y;
	int displacementX;
	int displacementY;

	Popup();
	void init();
	void tick();
	void open();
	void close();
	bool isDoneMoving();
	int getXOffset();
	int getYOffset();
	double getAlpha(double maxAlpha=1.);
	int getXDisplacement();
	int getYDisplacement();
};

struct SBarInfo
{
	enum MonospaceAlignment
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	TArray<FString> Images;
	SBarInfoMainBlock *huds[NUMHUDS];
	Popup popups[NUMPOPUPS];
	bool automapbar;
	bool interpolateHealth;
	bool interpolateArmor;
	bool completeBorder;
	bool lowerHealthCap;
	char spacingCharacter;
	TArray<std::pair<double, int>> protrusions;
	MonospaceAlignment spacingAlignment;
	int interpolationSpeed;
	int armorInterpolationSpeed;
	int height;
	int gameType;

	int _resW;
	int _resH;

	int GetGameType() { return gameType; }
	void ParseSBarInfo(int lump);
	void ParseMugShotBlock(FScanner &sc, FMugShotState &state);
	void ResetHuds();
	int newImage(const char* patchname);
	void Init();
	SBarInfo();
	SBarInfo(int lumpnum);
	~SBarInfo();

	static void	Load();
};

#define SCRIPT_CUSTOM	0
#define SCRIPT_DEFAULT	1
extern SBarInfo *SBarInfoScript[2];

#endif //__SBarInfo_SBAR_H__
