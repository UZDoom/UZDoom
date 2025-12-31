/*
** g_cvars.cpp
**
** collects all the CVARs that were scattered all across the code before
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

#include "c_cvars.h"
#include "g_levellocals.h"
#include "g_game.h"
#include "gstrings.h"
#include "i_system.h"
#include "v_font.h"
#include "utf8.h"
#include "gi.h"
#include "i_interface.h"

void I_UpdateWindowTitle();

CVAR (Bool, cl_spreaddecals, true, CVAR_ARCHIVE)
CVAR(Bool, var_pushers, true, CVAR_SERVERINFO);
CVAR(Bool, gl_cachenodes, true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR(Float, gl_cachetime, 0.6f, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR(Bool, alwaysapplydmflags, false, CVAR_SERVERINFO);

// [RH] Feature control cvars
CVAR(Bool, var_friction, true, CVAR_SERVERINFO);

CUSTOM_CVAR (Int, turnspeedwalkfast, 640, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
{
	if (self <= 0) self = 1;
}
CUSTOM_CVAR (Int, turnspeedsprintfast, 1280, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
{
	if (self <= 0) self = 1;
}
CUSTOM_CVAR (Int, turnspeedwalkslow, 320, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
{
	if (self <= 0) self = 1;
}
CUSTOM_CVAR (Int, turnspeedsprintslow, 320, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
{
	if (self <= 0) self = 1;
}



CUSTOM_CVAR (Bool, gl_lights, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG | CVAR_NOINITCALL)
{
	for (auto Level : AllLevels())
	{
		if (self) Level->RecreateAllAttachedLights();
		else Level->DeleteAllAttachedLights();
	}
}

CUSTOM_CVAR(Int, sv_corpsequeuesize, 64, CVAR_ARCHIVE|CVAR_SERVERINFO|CVAR_NOINITCALL)
{
	if (self > 0)
	{
		for (auto Level : AllLevels())
		{
			auto &corpsequeue = Level->CorpseQueue;
			while (corpsequeue.Size() > (unsigned)self)
			{
				AActor *corpse = corpsequeue[0];
				if (corpse) corpse->Destroy();
				corpsequeue.Delete(0);
			}
		}
	}
}

CUSTOM_CVAR (Int, cl_maxdecals, 1024, CVAR_ARCHIVE|CVAR_NOINITCALL)
{
	if (self < 0)
	{
		self = 0;
	}
	else for (auto Level : AllLevels())
	{
		while (Level->ImpactDecalCount > self)
		{
			DThinker *thinker = Level->FirstThinker(STAT_AUTODECAL);
			if (thinker != NULL)
			{
				thinker->Destroy();
				Level->ImpactDecalCount--;
			}
		}
	}
}


// [BC] Allow the maximum number of particles to be specified by a cvar (so people
// with lots of nice hardware can have lots of particles!).
CUSTOM_CVAR(Int, r_maxparticles, 4000, CVAR_ARCHIVE | CVAR_NOINITCALL)
{
	if (self == 0)
		self = 4000;
	else if (self > 65535)
		self = 65535;
	else if (self < 100)
		self = 100;

	if (gamestate != GS_STARTUP)
	{
		for (auto Level : AllLevels())
		{
			P_InitParticles(Level);
		}
	}
}

CUSTOM_CVAR(Float, teamdamage, 0.f, CVAR_SERVERINFO | CVAR_NOINITCALL)
{
	for (auto Level : AllLevels())
	{
		Level->teamdamage = self;
	}
}

CVAR(Float, cl_scaleweaponfov, 1.0f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
