/*
** g_hub.cpp
**
** Intermission stats for hubs
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#include "doomstat.h"
#include "g_hub.h"
#include "g_level.h"
#include "g_game.h"
#include "m_png.h"
#include "gstrings.h"
#include "wi_stuff.h"
#include "serializer.h"
#include "g_levellocals.h"


//==========================================================================
//
// Player is leaving the current level
//
//==========================================================================

struct FHubInfo
{
	int			levelnum;
	
	int			totalkills;
	int			maxkills;
	int			maxitems;
	int			maxsecret;
	int			maxfrags;

	wbplayerstruct_t	plyr[MAXPLAYERS];

	FHubInfo &operator=(const wbstartstruct_t &wbs)
	{
		levelnum	= wbs.finished_ep;
		totalkills	= wbs.totalkills;
		maxkills	= wbs.maxkills;
		maxsecret	= wbs.maxsecret;
		maxitems	= wbs.maxitems;
		maxfrags	= wbs.maxfrags;
		memcpy(plyr, wbs.plyr, sizeof(plyr));
		return *this;
	}
};


static TArray<FHubInfo> hubdata;

void G_LeavingHub(FLevelLocals *Level, int mode, cluster_info_t * cluster, wbstartstruct_t * wbs)
{
	unsigned int i, j;

	if (cluster->flags & CLUSTER_HUB)
	{
		for (i = 0; i < hubdata.Size(); i++)
		{
			if (hubdata[i].levelnum == Level->levelnum)
			{
				hubdata[i] = *wbs;
				break;
			}
		}
		if (i == hubdata.Size())
		{
			hubdata[hubdata.Reserve(1)] = *wbs;
		}

		hubdata[i].levelnum = Level->levelnum;
		if (!multiplayer && !deathmatch)
		{
			// The player counters don't work in hubs
			hubdata[i].plyr[0].skills = Level->killed_monsters;
			hubdata[i].plyr[0].sitems = Level->found_items;
			hubdata[i].plyr[0].ssecret = Level->found_secrets;
		}


		if (mode != FINISH_SameHub)
		{
			wbs->totalkills = Level->killed_monsters;
			wbs->maxkills = wbs->maxitems = wbs->maxsecret = 0;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				wbs->plyr[i].sitems = wbs->plyr[i].skills = wbs->plyr[i].ssecret = 0;
			}

			for (i = 0; i < hubdata.Size(); i++)
			{
				wbs->maxkills += hubdata[i].maxkills;
				wbs->maxitems += hubdata[i].maxitems;
				wbs->maxsecret += hubdata[i].maxsecret;
				for (j = 0; j < MAXPLAYERS; j++)
				{
					wbs->plyr[j].sitems += hubdata[i].plyr[j].sitems;
					wbs->plyr[j].skills += hubdata[i].plyr[j].skills;
					wbs->plyr[j].ssecret += hubdata[i].plyr[j].ssecret;
				}
			}
			if (cluster->ClusterName.IsNotEmpty())
			{
				if (cluster->flags & CLUSTER_LOOKUPNAME)
				{
					wbs->thisname = GStrings.GetString(cluster->ClusterName);
				}
				else
				{
					wbs->thisname = cluster->ClusterName;
					wbs->thisauthor = "";
				}
				wbs->LName0.SetInvalid();	// The level's own name was just invalidated, and so was its name patch.
			}
		}
	}
	if (mode != FINISH_SameHub) hubdata.Clear();
}

//==========================================================================
//
// Serialize intermission info for hubs
//
//==========================================================================

FSerializer &Serialize(FSerializer &arc, const char *key, wbplayerstruct_t &h, wbplayerstruct_t *def)
{
	if (arc.BeginObject(key))
	{
		arc("kills", h.skills)
			("items", h.sitems)
			("secrets", h.ssecret)
			("time", h.stime)
			("fragcount", h.fragcount)
			.Array("frags", h.frags, MAXPLAYERS)
			.EndObject();
	}
	return arc;
}

FSerializer &Serialize(FSerializer &arc, const char *key, FHubInfo &h, FHubInfo *def)
{
	if (arc.BeginObject(key))
	{
		arc("levelnum", h.levelnum)
			("totalkills", h.totalkills)
			("maxkills", h.maxkills)
			("maxitems", h.maxitems)
			("maxsecret", h.maxsecret)
			("maxfrags", h.maxfrags)
			.Array("players", h.plyr, MAXPLAYERS)
			.EndObject();
	}
	return arc;
}

void G_SerializeHub(FSerializer &arc)
{
	arc("hubinfo", hubdata);
}

void G_ClearHubInfo()
{
	hubdata.Clear();
}
