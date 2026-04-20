/*
** gi.h
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __GI_H__
#define __GI_H__

#include "basics.h"
#include "zstring.h"
#include "name.h"
#include "screenjob.h"

// Flags are not user configurable and only depend on the standard IWADs
enum
{
	GI_MAPxx				= 0x00000001,
	GI_SHAREWARE			= 0x00000002,
	GI_MENUHACK_EXTENDED	= 0x00000004,	// (Heretic)
	GI_TEASER2				= 0x00000008,	// Alternate version of the Strife Teaser
	GI_COMPATSHORTTEX		= 0x00000010,	// always force COMPAT_SHORTTEX for IWAD maps.
	GI_COMPATSTAIRS			= 0x00000020,	// same for stairbuilding
	GI_COMPATPOLY1			= 0x00000040,	// Hexen's MAP36 needs old polyobject drawing
	GI_COMPATPOLY2			= 0x00000080,	// so does HEXDD's MAP47
	GI_IGNORETITLEPATCHES	= 0x00000200,	// Ignore the map name graphics when not runnning in English language
	GI_NOSECTIONMERGE		= 0x00000400,	// For the original id IWADs: avoid merging sections due to how idbsp created its sectors.
};

#include "gametype.h"

extern const char *GameNames[17];

struct staticgameborder_t
{
	uint8_t offset;
	uint8_t size;
	char tl[8];
	char t[8];
	char tr[8];
	char l[8];
	char r[8];
	char bl[8];
	char b[8];
	char br[8];
};

struct gameborder_t
{
	uint8_t offset;
	uint8_t size;
	FString tl;
	FString t;
	FString tr;
	FString l;
	FString r;
	FString bl;
	FString b;
	FString br;

	gameborder_t &operator=(staticgameborder_t &other)
	{
		offset = other.offset;
		size = other.size;
		tl = other.tl;
		t = other.t;
		tr = other.tr;
		l = other.l;
		r = other.r;
		bl = other.bl;
		b = other.b;
		br = other.br;
		return *this;
	}
};

struct FGIFont
{
	FName fontname;
	FName color;
};

struct gameinfo_t
{
	int flags;
	EGameType gametype;
	FString ConfigName;

	FString TitlePage;
	bool nokeyboardcheats;
	bool drawreadthis;
	bool noloopfinalemusic;
	bool intermissioncounter;
	bool nightmarefast;
	bool swapmenu;
	bool dontcrunchcorpses;
	bool correctprintbold;
	bool forcetextinmenus;
	bool forcenogfxsubstitution;
	TArray<FName> creditPages;
	TArray<FName> finalePages;
	TArray<FName> infoPages;
	TArray<FName> DefaultWeaponSlots[10];
	TArray<FName> PlayerClasses;

	TArray<FName> PrecachedClasses;
	TArray<FString> PrecachedTextures;
	TArray<FSoundID> PrecachedSounds;
	TArray<FString> EventHandlers;

	FString titleMusic;
	int titleOrder;
	float titleTime;
	float advisoryTime;
	float pageTime;
	FString chatSound;
	FString finaleMusic;
	int finaleOrder;
	FString FinaleFlat;
	FString BorderFlat;
	FString SkyFlatName;
	FString ArmorIcon1;
	FString ArmorIcon2;
	FName BasicArmorClass;
	FName HexenArmorClass;
	FString PauseSign;
	bool UsePauseString;
	FString Endoom;
	double Armor2Percent;
	FString quitSound;
	gameborder_t Border;
	double telefogheight;
	int defKickback;
	FString translator;
	uint32_t defaultbloodcolor;
	uint32_t defaultbloodparticlecolor;
	FString statusbar;
	int statusbarfile = -1;
	FName statusbarclass;
	FName althudclass;
	int statusbarclassfile = -1;
	FName MessageBoxClass;
	FName HelpMenuClass;
	FName MenuDelegateClass;
	FName backpacktype;
	FString intermissionMusic;
	int intermissionOrder;
	FString CursorPic;
	uint32_t dimcolor;
	float dimamount;
	float bluramount;
	int definventorymaxamount;
	int defaultrespawntime;
	int defaultdropstyle;
	uint32_t pickupcolor;
	TArray<FString> quitmessages;
	FName mTitleColor;
	FName mFontColor;
	FName mFontColorValue;
	FName mFontColorMore;
	FName mFontColorHeader;
	FName mFontColorHighlight;
	FName mFontColorSelection;
	FName mSliderColor;
	FName mSliderBackColor;
	FString mBackButton;
	double gibfactor;
	int TextScreenX;
	int TextScreenY;
	FName DefaultConversationMenuClass;
	FName DefaultEndSequence;
	FString mMapArrow, mCheatMapArrow;
	FString mEasyKey, mCheatKey;
	FString Dialogue;
	TArray<FString> AddDialogues;
	FGIFont mStatscreenMapNameFont;
	FGIFont mStatscreenFinishedFont;
	FGIFont mStatscreenEnteringFont;
	FGIFont mStatscreenContentFont;
	FGIFont mStatscreenAuthorFont;
	bool norandomplayerclass;
	bool forcekillscripts;
	FName statusscreen_single;
	FName statusscreen_coop;
	FName statusscreen_dm;
	int healthpic;	// These get filled in from ALTHUDCF
	int berserkpic;
	double normforwardmove[2];
	double normsidemove[2];
	int fullscreenautoaspect = 3;
	bool nomergepickupmsg;
	bool mHideParTimes;
	CutsceneDef IntroScene;
	double BloodSplatDecalDistance;

	const char *GetFinalePage(unsigned int num) const;
};


extern gameinfo_t gameinfo;

inline const char *GameTypeName()
{
	return GameNames[gameinfo.gametype];
}

bool CheckGame(const char *string, bool chexisdoom);

#endif //__GI_H__
