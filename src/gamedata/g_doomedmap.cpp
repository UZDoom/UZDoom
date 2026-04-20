/*
** g_doomedmap.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2015-2016 Christoph Oelckers
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

#include "info.h"
#include "actor.h"
#include "p_lnspec.h"
#include "c_dispatch.h"
#include "v_text.h"
#include "engineerrors.h"


const char *SpecialMapthingNames[] = {
	"$Player1Start",
	"$Player2Start",
	"$Player3Start",
	"$Player4Start",
	"$Player5Start",
	"$Player6Start",
	"$Player7Start",
	"$Player8Start",
	"$DeathmatchStart",
	"$SSeqOverride",
	"$PolyAnchor",
	"$PolySpawn",
	"$PolySpawnCrush",
	"$PolySpawnHurt",
	"$SlopeFloorPointLine",
	"$SlopeCeilingPointLine",
	"$SetFloorSlope",
	"$SetCeilingSlope",
	"$VavoomFloor",
	"$VavoomCeiling",
	"$CopyFloorPlane",
	"$CopyCeilingPlane",
	"$VertexFloorZ",
	"$VertexCeilingZ",
	"$EDThing",

};
//==========================================================================
//
// Stuff that's only valid during definition time
//
//==========================================================================

struct MapinfoEdMapItem
{
	FName classname;	// DECORATE is read after MAPINFO so we do not have the actual classes available here yet.
	short special;
	signed char argsdefined;
	int args[5];
	// These are for error reporting. We must store the file information because it's no longer available when these items get resolved.
	FString filename;
	int linenum;
	bool noskillflags;
};

typedef TMap<int, MapinfoEdMapItem> IdMap;

static IdMap DoomEdFromMapinfo;

//==========================================================================
//
//
//==========================================================================

FDoomEdMap DoomEdMap;
// Set of spawnable things for the Thing_Spawn and Thing_Projectile specials.
FClassMap SpawnableThings;
FClassMap StrifeTypes;

static int sortnums (const void *a, const void *b)
{
	return (*(const FDoomEdMap::Pair**)a)->Key - (*(const FDoomEdMap::Pair**)b)->Key;
}

CCMD (dumpmapthings)
{
	TArray<FDoomEdMap::Pair*> infos(DoomEdMap.CountUsed());
	FDoomEdMap::Iterator it(DoomEdMap);
	FDoomEdMap::Pair *pair;

	while (it.NextPair(pair))
	{
		infos.Push(pair);
	}

	if (infos.Size () == 0)
	{
		Printf ("No map things registered\n");
	}
	else
	{
		qsort (&infos[0], infos.Size (), sizeof(FDoomEdMap::Pair*), sortnums);

		for (unsigned i = 0; i < infos.Size (); ++i)
		{
			if (infos[i]->Value.Type != NULL)
			{
				Printf("%6d %s\n", infos[i]->Key, infos[i]->Value.Type->TypeName.GetChars());
			}
			else if (infos[i]->Value.Special > 0)
			{
				Printf("%6d %s\n", infos[i]->Key, SpecialMapthingNames[infos[i]->Value.Special - 1]);
			}
			else
			{
				Printf("%6d none\n", infos[i]->Key);
			}

		}
	}
}


void FMapInfoParser::ParseDoomEdNums()
{
	TMap<int, bool> defined;
	int error = 0;

	MapinfoEdMapItem editem;

	editem.filename = sc.ScriptName;

	ParseOpenBrace();
	while (true)
	{
		if (sc.CheckString("}")) return;
		else if (sc.CheckNumber())
		{
			int ednum = sc.Number;
			sc.MustGetStringName("=");
			sc.MustGetString();

			bool *def = defined.CheckKey(ednum);
			if (def != NULL)
			{
				sc.ScriptMessage("Editor Number %d defined more than once", ednum);
				error++;
			}
			defined[ednum] = true;
			if (sc.String[0] == '$')
			{
				// add special stuff like playerstarts and sound sequence overrides here, too.
				editem.classname = NAME_None;
				editem.special = sc.MustMatchString(SpecialMapthingNames) + 1; // todo: assign proper constants
			}
			else
			{
				editem.classname = sc.String;
				editem.special = -1;
			}
			memset(editem.args, 0, sizeof(editem.args));
			editem.argsdefined = 0;
			editem.noskillflags = false;
			editem.linenum = sc.Line;

			int minargs = 0;
			int maxargs = 5;
			FString specialname;
			if (sc.CheckString(","))
			{
				if (sc.CheckString("noskillflags"))
				{
					editem.noskillflags = true;
					if (!sc.CheckString(",")) goto noargs;
				}
				editem.argsdefined = 5; // mark args as used - if this is done we need to prevent assignment of map args in P_SpawnMapThing.
				if (editem.special < 0) editem.special = 0;
				if (!sc.CheckNumber())
				{
					sc.MustGetString();
					specialname = sc.String;	// save for later error reporting.
					editem.special = P_FindLineSpecial(sc.String, &minargs, &maxargs);
					if (editem.special == 0 || minargs == -1)
					{
						sc.ScriptMessage("Invalid special %s for Editor Number %d", sc.String, ednum);
						error++;
						minargs = 0;
						maxargs = 5;
					}
					if (!sc.CheckString(","))
					{
						// special case: Special without arguments
						if (minargs != 0)
						{
							sc.ScriptMessage("Incorrect number of args for special %s, min = %d, max = %d, found = 0", specialname.GetChars(), minargs, maxargs);
							error++;
						}
						DoomEdFromMapinfo.Insert(ednum, editem);
						continue;
					}
					sc.MustGetNumber();
				}
				int i = 0;
				while (i < 5)
				{
					editem.args[i] = sc.Number;
					i++;
					if (!sc.CheckString(",")) break;
					// special check for the ambient sounds which combine the arg being set here with the ones on the mapthing.
					if (sc.CheckString("+"))
					{
						editem.argsdefined = i;
						break;
					}
					sc.MustGetNumber();

				}
				if (specialname.IsNotEmpty() && (i < minargs || i > maxargs))
				{
					sc.ScriptMessage("Incorrect number of args for special %s, min = %d, max = %d, found = %d", specialname.GetChars(), minargs, maxargs, i);
					error++;
				}
			}
		noargs:
			DoomEdFromMapinfo.Insert(ednum, editem);
		}
		else
		{
			sc.ScriptError("Number expected");
		}
	}
	if (error > 0)
	{
		sc.ScriptError("%d errors encountered in DoomEdNum definition", error);
	}
}

void InitActorNumsFromMapinfo()
{
	DoomEdMap.Clear();
	IdMap::Iterator it(DoomEdFromMapinfo);
	IdMap::Pair *pair;
	int error = 0;

	while (it.NextPair(pair))
	{
		PClassActor *cls = NULL;
		if (pair->Value.classname != NAME_None)
		{
			cls = PClass::FindActor(pair->Value.classname);
			if (cls == NULL)
			{
				Printf(TEXTCOLOR_RED "Script error, \"%s\" line %d:\nUnknown actor class %s\n",
					pair->Value.filename.GetChars(), pair->Value.linenum, pair->Value.classname.GetChars());
				error++;
			}
		}
		FDoomEdEntry ent;
		ent.Type = cls;
		ent.Special = pair->Value.special;
		ent.ArgsDefined = pair->Value.argsdefined;
		ent.NoSkillFlags = pair->Value.noskillflags;
		memcpy(ent.Args, pair->Value.args, sizeof(ent.Args));
		DoomEdMap.Insert(pair->Key, ent);
	}
	if (error > 0)
	{
		I_Error("%d unknown actor classes found", error);
	}
	DoomEdFromMapinfo.Clear();	// we do not need this any longer
}

PClassActor *P_GetSpawnableType(int spawnnum)
{
	if (spawnnum < 0)
	{ // A named arg from a UDMF map
		FName spawnname = FName(ENamedName(-spawnnum));
		if (spawnname.IsValidName())
		{
			return PClass::FindActor(spawnname);
		}
	}
	else
	{ // A numbered arg from a Hexen or UDMF map
		PClassActor **type = SpawnableThings.CheckKey(spawnnum);
		if (type != NULL)
		{
			return *type;
		}
	}
	return NULL;
}

struct MapinfoSpawnItem
{
	FName classname;	// DECORATE is read after MAPINFO so we do not have the actual classes available here yet.
	// These are for error reporting. We must store the file information because it's no longer available when these items get resolved.
	FString filename;
	int linenum;
};

typedef TMap<int, MapinfoSpawnItem> SpawnMap;
static SpawnMap SpawnablesFromMapinfo;
static SpawnMap ConversationIDsFromMapinfo;

static int SpawnableSort(const void *a, const void *b)
{
	return (*((FClassMap::Pair **)a))->Key - (*((FClassMap::Pair **)b))->Key;
}

static void DumpClassMap(FClassMap &themap)
{
	FClassMap::Iterator it(themap);
	FClassMap::Pair *pair;
	TArray<FClassMap::Pair*> allpairs(themap.CountUsed(), true);
	int i = 0;

	// Sort into numerical order, since their arrangement in the map can
	// be in an unspecified order.
	while (it.NextPair(pair))
	{
		allpairs[i++] = pair;
	}
	qsort(allpairs.Data(), i, sizeof(allpairs[0]), SpawnableSort);
	for (int j = 0; j < i; ++j)
	{
		pair = allpairs[j];
		Printf ("%d %s\n", pair->Key, pair->Value->TypeName.GetChars());
	}
}

CCMD(dumpspawnables)
{
	DumpClassMap(SpawnableThings);
}

CCMD (dumpconversationids)
{
	DumpClassMap(StrifeTypes);
}

void ClearStrifeTypes()
{
	StrifeTypes.Clear();
}


static void ParseSpawnMap(FScanner &sc, SpawnMap & themap, const char *descript)
{
	TMap<int, bool> defined;
	int error = 0;

	MapinfoSpawnItem editem;

	editem.filename = sc.ScriptName;

	while (true)
	{
		if (sc.CheckString("}")) return;
		else if (sc.CheckNumber())
		{
			int ednum = sc.Number;
			sc.MustGetStringName("=");
			sc.MustGetString();

			bool *def = defined.CheckKey(ednum);
			if (def != NULL)
			{
				sc.ScriptMessage("%s %d defined more than once", descript, ednum);
				error++;
			}
			else if (ednum < 0)
			{
				sc.ScriptMessage("%s must be positive, got %d", descript, ednum);
				error++;
			}
			defined[ednum] = true;
			editem.classname = sc.String;
			editem.linenum = sc.Line;

			themap.Insert(ednum, editem);
		}
		else
		{
			sc.ScriptError("Number expected");
		}
	}
	if (error > 0)
	{
		sc.ScriptError("%d errors encountered in %s definition", error, descript);
	}
}

void FMapInfoParser::ParseSpawnNums()
{
	ParseOpenBrace();
	ParseSpawnMap(sc, SpawnablesFromMapinfo, "Spawn number");
}

void FMapInfoParser::ParseConversationIDs()
{
	ParseOpenBrace();
	ParseSpawnMap(sc, ConversationIDsFromMapinfo, "Conversation ID");
}


void InitClassMap(FClassMap &themap, SpawnMap &thedata)
{
	themap.Clear();
	SpawnMap::Iterator it(thedata);
	SpawnMap::Pair *pair;
	int error = 0;

	while (it.NextPair(pair))
	{
		PClassActor *cls = NULL;
		if (pair->Value.classname != NAME_None)
		{
			cls = PClass::FindActor(pair->Value.classname);
			if (cls == NULL)
			{
				Printf(TEXTCOLOR_RED "Script error, \"%s\" line %d:\nUnknown actor class %s\n",
					pair->Value.filename.GetChars(), pair->Value.linenum, pair->Value.classname.GetChars());
				error++;
			}
			themap.Insert(pair->Key, cls);
		}
		else
		{
			themap.Remove(pair->Key);
		}
	}
	if (error > 0)
	{
		I_Error("%d unknown actor classes found", error);
	}
	thedata.Clear();	// we do not need this any longer
}

void InitSpawnablesFromMapinfo()
{
	InitClassMap(SpawnableThings, SpawnablesFromMapinfo);
	InitClassMap(StrifeTypes, ConversationIDsFromMapinfo);
}
