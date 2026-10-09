/*
** g_dumpinfo.cpp
**
** diagnostic CCMDs that output info about the current game
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2003-2016 Christoph Oelckers
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

// IWYU pragma: no_include "fs_decompress.h"
// IWYU pragma: no_include "info.h"

#include "a_dynlight.h"
#include "a_sharedglobal.h"
#include "actor.h"
#include "basics.h"
#include "c_commandline.h"
#include "c_dispatch.h"
#include "c_functions.h"
#include "cmdlib.h"
#include "d_main.h"
#include "d_net.h"
#include "d_player.h"
#include "d_protocol.h"
#include "dobjgc.h"
#include "doomstat.h"
#include "dthinker.h"
#include "filesystem.h"
#include "fs_filesystem.h"
#include "g_level.h"
#include "g_levellocals.h"
#include "g_mapinfo.h"
#include "gametexture.h"
#include "name.h"
#include "p_3dfloors.h"
#include "p_setup.h"
#include "p_tags.h"
#include "portal.h"
#include "printf.h"
#include "r_defs.h"
#include "r_interpolate.h"
#include "r_state.h"
#include "sprites.h"
#include "statnums.h"
#include "stats.h"
#include "tarray.h"
#include "textureid.h"
#include "texturemanager.h"
#include "vectors.h"
#include "zstring.h"

//==========================================================================
//
// CCMDs
//
//==========================================================================

CCMD(listlights)
{
	int walls, sectors;
	int allwalls=0, allsectors=0, allsubsecs = 0;
	int i=0, shadowcount = 0;
	FDynamicLight * dl;

	for (auto Level : AllLevels())
	{
		Printf("Lights for %s\n", Level->MapName.GetChars());
		for (dl = Level->lights; dl; dl = dl->next)
		{
			walls=0;
			sectors=0;
			Printf("%s at (%f, %f, %f), color = 0x%02x%02x%02x, radius = %f %s %s",
				   dl->target->GetClass()->TypeName.GetChars(),
				   dl->X(), dl->Y(), dl->Z(), dl->GetRed(), dl->GetGreen(), dl->GetBlue(),
				   dl->radius, dl->IsAttenuated()? "attenuated" : "", dl->shadowmapped? "shadowmapped" : "");
			i++;
			shadowcount += dl->shadowmapped;

			if (dl->target)
			{
				FTextureID spr = sprites[dl->target->sprite].GetSpriteFrame(dl->target->frame, 0, nullAngle, nullptr);
				Printf(", frame = %s\n", TexMan.GetGameTexture(spr)->GetName().GetChars());
			}

			/*
			Printf("- %d walls, %d sectors\n", walls, sectors);
			*/

		}
		Printf("%i dynamic lights, %d shadowmapped, %d walls, %d sectors\n\n\n", i, shadowcount, allwalls, allsectors);
	}
}

CCMD (countdecals)
{
	for (auto Level : AllLevels())
	{
		auto iterator = Level->GetThinkerIterator<DImpactDecal>(NAME_None, STAT_AUTODECAL);
		int count = 0;

		while (iterator.Next())
			count++;

		Printf("%s: Counted %d impact decals, level counter is at %d\n", Level->MapName.GetChars(), count, Level->ImpactDecalCount);
	}
}

CCMD (spray)
{
	if (players[consoleplayer].mo == NULL || argv.argc() < 2)
	{
		Printf ("Usage: spray <decal>\n");
		return;
	}

	Net_WriteInt8 (DEM_SPRAY);
	Net_WriteString (argv[1]);
}

//==========================================================================
//
// CCMD mapchecksum
//
//==========================================================================

CCMD (mapchecksum)
{
	auto printmap = [](const char *name)
	{
		if (name[0] == '*')
		{
			name = level.MapName.GetChars();
		}

		MapData *map = P_OpenMapData(name, true);
		uint8_t sum[16];
		int lump = 0;
		if (map)
		{
			map->GetChecksum(sum);
			lump = map->lumpnum;
			delete map;
		}
		else if (name[0] != '\0')
		{
			Printf("Cannot load %s as a map\n", name);
			return;
		}

		Printf(
			"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X // %s %s\n",
			sum[0], sum[1], sum[2], sum[3], sum[4], sum[5], sum[6], sum[7],
			sum[8], sum[9], sum[10], sum[11], sum[12], sum[13], sum[14], sum[15],
			fileSystem.GetResourceFileName(fileSystem.GetFileContainer(lump)),
			name
		);
	};

	if (argv.argc() == 1)
	{
		printmap("*");
	}
	else
	{
		for (int i = 1; i < argv.argc(); ++i)
		{
			printmap(argv[i]);
		}
	}
}

//==========================================================================
//
// CCMD hiddencompatflags
//
//==========================================================================

CCMD (hiddencompatflags)
{
	for(auto Level : AllLevels())
	{
		Printf(
			"%s: %08x %08x %08x\n",
			Level->MapName.GetChars(),
			static_cast<uint32_t>(Level->ii_compatflags),
			static_cast<uint32_t>(Level->ii_compatflags2),
			static_cast<uint32_t>(Level->ib_compatflags)
		);
	}
}

CCMD(dumpportals)
{
	for (auto Level : AllLevels())
	{
		Printf("Portal groups for %s\n", Level->MapName.GetChars());
		for (unsigned i = 0; i < Level->portalGroups.Size(); i++)
		{
			auto p = Level->portalGroups[i];
			double xdisp = p->mDisplacement.X;
			double ydisp = p->mDisplacement.Y;
			Printf(PRINT_LOG, "Portal #%d, %s, displacement = (%f,%f)\n", i, p->plane == 0 ? "floor" : "ceiling",
				   xdisp, ydisp);
			Printf(PRINT_LOG, "Coverage:\n");
			for (auto &sub : Level->subsectors)
			{
				auto port = sub.render_sector->GetPortalGroup(p->plane);
				if (port == p)
				{
					Printf(PRINT_LOG, "\tSubsector %d (%d):\n\t\t", sub.Index(), sub.render_sector->sectornum);
					for (unsigned k = 0; k < sub.numlines; k++)
					{
						Printf(PRINT_LOG, "(%.3f,%.3f), ", sub.firstline[k].v1->fX() + xdisp, sub.firstline[k].v1->fY() + ydisp);
					}
					Printf(PRINT_LOG, "\n\t\tCovered by subsectors:\n");
					FPortalCoverage *cov = &sub.portalcoverage[p->plane];
					for (int l = 0; l < cov->sscount; l++)
					{
						subsector_t *csub = &Level->subsectors[cov->subsectors[l]];
						Printf(PRINT_LOG, "\t\t\t%5d (%4d): ", cov->subsectors[l], csub->render_sector->sectornum);
						for (unsigned m = 0; m < csub->numlines; m++)
						{
							Printf(PRINT_LOG, "(%.3f,%.3f), ", csub->firstline[m].v1->fX(), csub->firstline[m].v1->fY());
						}
						Printf(PRINT_LOG, "\n");
					}
				}
			}
		}
	}
}

ADD_STAT (interpolations)
{
	FString out;
	for (auto Level : AllLevels())
	{
		if (out.Len() > 0) out << '\n';
		out.AppendFormat("%s: %d interpolations", Level->MapName.GetChars(), Level->interpolator.CountInterpolations ());

	}
	return out;
}

CCMD(printsections)
{
	void PrintSections(FLevelLocals *Level);
	for (auto Level : AllLevels())
	{
		PrintSections(Level);
	}
}

CCMD(dumptags)
{
	for (auto Level : AllLevels())
	{
		Level->tagManager.DumpTags();
	}
}

CCMD(dump3df)
{
	if (argv.argc() > 1)
	{
		// Print 3D floor info for a single sector.
		// This only checks the primary level.
		int sec = (int)strtoll(argv[1], NULL, 10);
		if ((unsigned)sec >= primaryLevel->sectors.Size())
		{
			Printf("Sector %d does not exist.\n", sec);
			return;
		}
		sector_t *sector = &primaryLevel->sectors[sec];
		TArray<F3DFloor*> & ffloors = sector->e->XFloor.ffloors;

		for (unsigned int i = 0; i < ffloors.Size(); i++)
		{
			double height = ffloors[i]->top.plane->ZatPoint(sector->centerspot);
			double bheight = ffloors[i]->bottom.plane->ZatPoint(sector->centerspot);

			Printf("FFloor %d @ top = %f (model = %d), bottom = %f (model = %d), flags = %B, alpha = %d %s %s\n",
				i, height, ffloors[i]->top.model->sectornum,
				bheight, ffloors[i]->bottom.model->sectornum,
				ffloors[i]->flags, ffloors[i]->alpha, (ffloors[i]->flags&FF_EXISTS) ? "Exists" : "", (ffloors[i]->flags&FF_DYNAMIC) ? "Dynamic" : "");
		}
	}
}

//============================================================================
//
// print the group link table to the console
//
//============================================================================

CCMD(dumplinktable)
{
	for (auto Level : AllLevels())
	{
		Printf("Portal displacements for %s:\n", Level->MapName.GetChars());
		for (int x = 1; x < Level->Displacements.size; x++)
		{
			for (int y = 1; y < Level->Displacements.size; y++)
			{
				FDisplacement &disp = Level->Displacements(x, y);
				Printf("%c%c(%6d, %6d)", TEXTCOLOR_ESCAPE, 'C' + disp.indirect, int(disp.pos.X), int(disp.pos.Y));
			}
			Printf("\n");
		}
	}
}


//===========================================================================
//
// CCMD printinv
//
// Prints the console player's current inventory.
//
//===========================================================================

CCMD(printinv)
{
	unsigned int pnum = consoleplayer;

#ifdef _DEBUG
	// Only allow peeking on other players' inventory in debug builds.
	if (argv.argc() > 1)
	{
		pnum = atoi(argv[1]);
		if (pnum < 0 || pnum >= MAXPLAYERS)
		{
			return;
		}
	}
#endif
	C_PrintInv(players[pnum].mo);
}

CCMD(targetinv)
{
	FTranslatedLineTarget t;

	if (CheckCheatmode() || players[consoleplayer].mo == NULL)
		return;

	C_AimLine(&t, true);

	if (t.linetarget)
	{
		C_PrintInv(t.linetarget);
	}
	else Printf("No target found. Targetinv cannot find actors that have "
		"the NOBLOCKMAP flag or have height/radius of 0.\n");
}


//==========================================================================
//
// Lists all currently defined maps
//
//==========================================================================

CCMD(listmaps)
{
	int iwadNum = fileSystem.GetIwadNum();

	for (unsigned i = 0; i < wadlevelinfos.Size(); i++)
	{
		level_info_t *info = &wadlevelinfos[i];
		MapData *map = P_OpenMapData(info->MapName.GetChars(), true);

		if (map != NULL)
		{
			int mapWadNum = fileSystem.GetFileContainer(map->lumpnum);

			if (argv.argc() == 1
			    || CheckWildcards(argv[1], info->MapName.GetChars())
			    || CheckWildcards(argv[1], info->LookupLevelName().GetChars())
			    || CheckWildcards(argv[1], fileSystem.GetResourceFileName(mapWadNum)))
			{
				bool isFromPwad = mapWadNum != iwadNum;

				const char* lineColor = isFromPwad ? TEXTCOLOR_LIGHTBLUE : "";

				Printf("%s%s: '%s' (%s)\n", lineColor, info->MapName.GetChars(),
					info->LookupLevelName().GetChars(),
					fileSystem.GetResourceFileName(mapWadNum));
			}
			delete map;
		}
	}
}

//==========================================================================
//
// For testing sky fog sheets
//
//==========================================================================
CCMD(skyfog)
{
	if (argv.argc() > 1)
	{
		// Do this only on the primary level.
		primaryLevel->skyfog = max(0, (int)strtoull(argv[1], NULL, 0));
	}
	Printf("%d\n", primaryLevel->skyfog);
}


//==========================================================================
//
//
//==========================================================================

CCMD(listsnapshots)
{
	for (unsigned i = 0; i < wadlevelinfos.Size(); ++i)
	{
		FCompressedBuffer *snapshot = &wadlevelinfos[i].Snapshot;
		if (snapshot->mBuffer != nullptr)
		{
			Printf("%s (%lu -> %lu bytes)\n", wadlevelinfos[i].MapName.GetChars(), snapshot->mCompressedSize, snapshot->mSize);
		}
	}
}
