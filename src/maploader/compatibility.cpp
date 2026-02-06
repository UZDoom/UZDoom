/*
** compatibility.cpp
**
** Handles compatibility flags for maps that are unlikely to be updated.
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
** Copyright 2009-2018 Christoph Oelckers
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
** This file is for maps that have been rendered broken by bug fixes or other
** changes that seemed minor at the time, and it is unlikely that the maps
** will be changed. If you are making a map and you know it needs a
** compatibility option to play properly, you are advised to specify so with
** a MAPINFO.
*/

// HEADER FILES ------------------------------------------------------------

#include "actor.h"
#include "doomdef.h"
#include "doomstat.h"
#include "filesystem.h"
#include "g_levellocals.h"
#include "gi.h"
#include "maploader/maploader.h"
#include "p_lnspec.h"
#include "p_setup.h"
#include "p_tags.h"
#include "sc_man.h"
#include "textures.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

union FMD5Holder
{
	uint8_t Bytes[16];
	uint32_t DWords[4];
	hash_t Hash;
};

struct FCompatValues
{
	ELevelCompatFlags Flags1;
	ELevelCompatFlags2 Flags2;
	ELevelBugCompatFlags BugCompatFlags;
	unsigned int ExtCommandIndex;
};

struct FMD5HashTraits
{
	hash_t Hash(const FMD5Holder key)
	{
		return key.Hash;
	}
	int Compare(const FMD5Holder left, const FMD5Holder right)
	{
		return left.DWords[0] != right.DWords[0] ||
			left.DWords[1] != right.DWords[1] ||
			left.DWords[2] != right.DWords[2] ||
			left.DWords[3] != right.DWords[3];
	}
};

struct FCompatOption
{
	const char *Name;
	uint32_t CompatFlags;
	int WhichSlot;
};

enum ECompatSlot
{
	SLOT_COMPAT,
	SLOT_COMPAT2,
	SLOT_BCOMPAT,
	COMPATSLOT_COUNT
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

static TMap<FMD5Holder, FCompatValues, FMD5HashTraits> BCompatMap;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static FCompatOption Options[] =
{
	{ "setslopeoverflow",		BCOMPATF_SETSLOPEOVERFLOW, SLOT_BCOMPAT },
	{ "resetplayerspeed",		BCOMPATF_RESETPLAYERSPEED, SLOT_BCOMPAT },
	{ "vileghosts",				COMPATF_VILEGHOSTS, SLOT_COMPAT },
	{ "ignoreteleporttags",		BCOMPATF_BADTELEPORTERS, SLOT_BCOMPAT },
	{ "rebuildnodes",			BCOMPATF_REBUILDNODES, SLOT_BCOMPAT },
	{ "linkfrozenprops",		BCOMPATF_LINKFROZENPROPS, SLOT_BCOMPAT },
	{ "floatbob",				BCOMPATF_FLOATBOB, SLOT_BCOMPAT },
	{ "noslopeid",				BCOMPATF_NOSLOPEID, SLOT_BCOMPAT },
	{ "clipmidtex",				BCOMPATF_CLIPMIDTEX, SLOT_BCOMPAT },
	{ "nosectionmerge",			BCOMPATF_NOSECTIONMERGE, SLOT_BCOMPAT },
	{ "nomirrors",				BCOMPATF_NOMIRRORS, SLOT_BCOMPAT },


	// list copied from g_mapinfo.cpp
	{ "shorttex",				COMPATF_SHORTTEX, SLOT_COMPAT },
	{ "stairs",					COMPATF_STAIRINDEX, SLOT_COMPAT },
	{ "limitpain",				COMPATF_LIMITPAIN, SLOT_COMPAT },
	{ "nopassover",				COMPATF_NO_PASSMOBJ, SLOT_COMPAT },
	{ "notossdrops",			COMPATF_NOTOSSDROPS, SLOT_COMPAT },
	{ "useblocking", 			COMPATF_USEBLOCKING, SLOT_COMPAT },
	{ "nodoorlight",			COMPATF_NODOORLIGHT, SLOT_COMPAT },
	{ "ravenscroll",			COMPATF_RAVENSCROLL, SLOT_COMPAT },
	{ "soundtarget",			COMPATF_SOUNDTARGET, SLOT_COMPAT },
	{ "dehhealth",				COMPATF_DEHHEALTH, SLOT_COMPAT },
	{ "trace",					COMPATF_TRACE, SLOT_COMPAT },
	{ "dropoff",				COMPATF_DROPOFF, SLOT_COMPAT },
	{ "boomscroll",				COMPATF_BOOMSCROLL, SLOT_COMPAT },
	{ "invisibility",			COMPATF_INVISIBILITY, SLOT_COMPAT },
	{ "silentinstantfloors",	COMPATF_SILENT_INSTANT_FLOORS, SLOT_COMPAT },
	{ "sectorsounds",			COMPATF_SECTORSOUNDS, SLOT_COMPAT },
	{ "missileclip",			COMPATF_MISSILECLIP, SLOT_COMPAT },
	{ "crossdropoff",			COMPATF_CROSSDROPOFF, SLOT_COMPAT },
	{ "wallrun",				COMPATF_WALLRUN, SLOT_COMPAT },		// [GZ] Added for CC MAP29
	{ "anybossdeath",			COMPATF_ANYBOSSDEATH, SLOT_COMPAT },// [GZ] Added for UAC_DEAD
	{ "mushroom",				COMPATF_MUSHROOM, SLOT_COMPAT },
	{ "mbfmonstermove",			COMPATF_MBFMONSTERMOVE, SLOT_COMPAT },
	{ "noblockfriends",			COMPATF_NOBLOCKFRIENDS, SLOT_COMPAT },
	{ "spritesort",				COMPATF_SPRITESORT, SLOT_COMPAT },
	{ "hitscan",				COMPATF_HITSCAN, SLOT_COMPAT },
	{ "lightlevel",				COMPATF_LIGHT, SLOT_COMPAT },
	{ "polyobj",				COMPATF_POLYOBJ, SLOT_COMPAT },
	{ "maskedmidtex",			COMPATF_MASKEDMIDTEX, SLOT_COMPAT },
	{ "badangles",				COMPATF2_BADANGLES, SLOT_COMPAT2 },
	{ "floormove",				COMPATF2_FLOORMOVE, SLOT_COMPAT2 },
	{ "soundcutoff",			COMPATF2_SOUNDCUTOFF, SLOT_COMPAT2 },
	{ "pointonline",			COMPATF2_POINTONLINE, SLOT_COMPAT2 },
	{ "multiexit",				COMPATF2_MULTIEXIT, SLOT_COMPAT2 },
	{ "teleport",				COMPATF2_TELEPORT, SLOT_COMPAT2 },
	{ "disablepushwindowcheck",	COMPATF2_PUSHWINDOW, SLOT_COMPAT2 },
	{ "checkswitchrange",		COMPATF2_CHECKSWITCHRANGE, SLOT_COMPAT2 },
	{ "explode1",				COMPATF2_EXPLODE1, SLOT_COMPAT2 },
	{ "explode2",				COMPATF2_EXPLODE2, SLOT_COMPAT2 },
	{ "railing",				COMPATF2_RAILING, SLOT_COMPAT2 },
	{ "scriptwait",				COMPATF2_SCRIPTWAIT, SLOT_COMPAT2 },
	{ "nombf21",				COMPATF2_NOMBF21, SLOT_COMPAT2 },
	{ "voodoozombies",			COMPATF2_VOODOO_ZOMBIES, SLOT_COMPAT2 },
	{ "fdteleport",				COMPATF2_FDTELEPORT, SLOT_COMPAT2 },
	{ "noacsargcheck",			COMPATF2_NOACSARGCHECK, SLOT_COMPAT2 },
	{ "novdolllockmsg",			COMPATF2_NOVDOLLLOCKMSG, SLOT_COMPAT2 },
	{ NULL, 0, 0 }
};

static const char *const LineSides[] =
{
	"Front", "Back", NULL
};

static const char *const WallTiers[] =
{
	"Top", "Mid", "Bot", NULL
};

static const char *const SectorPlanes[] =
{
	"floor", "ceil", NULL
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// ParseCompatibility
//
//==========================================================================

void ParseCompatibility()
{
	TArray<FMD5Holder> md5array;
	FMD5Holder md5;
	FCompatValues flags;
	int i, x;
	unsigned int j;

	BCompatMap.Clear();

	// The contents of this file are not cumulative, as it should not
	// be present in user-distributed maps.
	FScanner sc(fileSystem.GetNumForFullName("compatibility.txt"));

	while (sc.GetString())	// Get MD5 signature
	{
		do
		{
			if (strlen(sc.String) != 32)
			{
				sc.ScriptError("MD5 signature must be exactly 32 characters long");
			}
			for (i = 0; i < 32; ++i)
			{
				if (sc.String[i] >= '0' && sc.String[i] <= '9')
				{
					x = sc.String[i] - '0';
				}
				else
				{
					sc.String[i] |= 'a' ^ 'A';
					if (sc.String[i] >= 'a' && sc.String[i] <= 'f')
					{
						x = sc.String[i] - 'a' + 10;
					}
					else
					{
						x = 0;
						sc.ScriptError("MD5 signature must be a hexadecimal value");
					}
				}
				if (!(i & 1))
				{
					md5.Bytes[i / 2] = x << 4;
				}
				else
				{
					md5.Bytes[i / 2] |= x;
				}
			}
			md5array.Push(md5);
			sc.MustGetString();
		} while (!sc.Compare("{"));
		
		flags.Flags1 = 0;
		flags.Flags2 = 0;
		flags.BugCompatFlags = 0;
		flags.ExtCommandIndex = ~0u;

		while (sc.GetString())
		{
			if ((i = sc.MatchString(&Options[0].Name, sizeof(*Options))) >= 0)
			{
				switch (ECompatSlot(Options[i].WhichSlot))
				{
					case SLOT_COMPAT: flags.Flags1 |= ELevelCompatFlags::FromInt(Options[i].CompatFlags); break;
					case SLOT_COMPAT2: flags.Flags2 |= ELevelCompatFlags2::FromInt(Options[i].CompatFlags); break;
					case SLOT_BCOMPAT: flags.BugCompatFlags |= ELevelBugCompatFlags::FromInt(Options[i].CompatFlags); break;
					case COMPATSLOT_COUNT: /* noop */ break;
				}
			}
			else
			{
				sc.UnGet();
				break;
			}
		}
		sc.MustGetStringName("}");
		for (j = 0; j < md5array.Size(); ++j)
		{
			BCompatMap[md5array[j]] = flags;
		}
		md5array.Clear();
	}
}

//==========================================================================
//
// CheckCompatibility
//
//==========================================================================

FName MapLoader::CheckCompatibility(MapData *map)
{
	FMD5Holder md5;
	FCompatValues *flags;

	if (BCompatMap.CountUsed() == 0) ParseCompatibility();

	Level->ii_compatflags = 0;
	Level->ii_compatflags2 = 0;
	Level->ib_compatflags = 0;

	// When playing Doom IWAD levels force BCOMPATF_NOSECTIONMERGE, COMPAT_SHORTTEX and COMPATF_LIGHT.
	// I'm not sure if the IWAD maps actually need COMPATF_LIGHT but it certainly does not hurt.
	// TNT's MAP31 also needs COMPATF_STAIRINDEX but that only gets activated for TNT.WAD.
	if (fileSystem.GetFileContainer(map->lumpnum) == fileSystem.GetIwadNum())
	{
		if ((gameinfo.flags & GI_COMPATSHORTTEX) && Level->maptype == MAPTYPE_DOOM)
		{
			Level->ii_compatflags = (COMPATF_SHORTTEX | COMPATF_LIGHT);
			if (gameinfo.flags & GI_COMPATSTAIRS) Level->ii_compatflags |= COMPATF_STAIRINDEX;
		}
		if (gameinfo.flags & GI_NOSECTIONMERGE)
		{
			//Level->ib_compatflags |= BCOMPATF_NOSECTIONMERGE;
		}
	}

	map->GetChecksum(md5.Bytes);

	flags = BCompatMap.CheckKey(md5);

	FString hash;

	for (size_t j = 0; j < sizeof(md5.Bytes); ++j)
	{
		hash.AppendFormat("%02X", md5.Bytes[j]);
	}

	if (developer >= DMSG_NOTIFY)
	{
		Printf("MD5 = %s", hash.GetChars());
		if (flags != NULL)
		{
			Printf(", cflags = %08x, cflags2 = %08x, bflags = %08x\n",
				flags->Flags1, flags->Flags2, flags->BugCompatFlags);
		}
		else
		{
			Printf("\n");
		}
	}

	if (flags != NULL)
	{
		Level->ii_compatflags |= flags->Flags1;
		Level->ii_compatflags2 |= flags->Flags2;
		Level->ib_compatflags |= flags->BugCompatFlags;
	}

	// Reset i_compatflags
	Level->ApplyCompatibility();
	Level->ApplyCompatibility2();
	// Set floatbob compatibility for all maps with an original Hexen MAPINFO.
	if (Level->flags2 & LEVEL2_HEXENHACK)
	{
		Level->ib_compatflags |= BCOMPATF_FLOATBOB;
	}
	return FName(hash, true);	// if this returns NAME_None it means there is no scripted compatibility handler.
}
