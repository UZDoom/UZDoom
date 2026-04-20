/*
** c_functions.cpp
**
** Miscellaneous console command helper functions.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2003-2016 Christoph Oelckers
** Copyright 2016 Rachael Alexanderson
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

#include "d_player.h"
#include "p_local.h"

#include "c_functions.h"

void C_PrintInfo(AActor *target, bool verbose)
{
	if (target->player)
		Printf("Player=%s, ", target->player->userinfo.GetName());
	Printf("Class=%s, Health=%d, Spawnhealth=%d\n",
		target->GetClass()->TypeName.GetChars(),
		target->health,
		target->SpawnHealth());
	if (verbose) PrintMiscActorInfo(target);
}

void C_AimLine(FTranslatedLineTarget *t, bool nonshootable)
{
	P_AimLineAttack(players[consoleplayer].mo,players[consoleplayer].mo->Angles.Yaw, MISSILERANGE, t, nullAngle,
		(nonshootable) ? ALF_CHECKNONSHOOTABLE|ALF_FORCENOSMART|ALF_IGNORENOAUTOAIM : ALF_IGNORENOAUTOAIM);
}

void C_PrintInv(AActor *target)
{
	AActor *item;
	int count = 0;

	if (target == NULL)
	{
		Printf("No target found!\n");
		return;
	}

	if (target->player)
		Printf("Inventory for Player '%s':\n", target->player->userinfo.GetName());
	else
		Printf("Inventory for Target '%s':\n", target->GetClass()->TypeName.GetChars());

	for (item = target->Inventory; item != NULL; item = item->Inventory)
	{
		Printf ("    %s #%u (%d/%d)\n", item->GetClass()->TypeName.GetChars(),
			item->InventoryID,
			item->IntVar(NAME_Amount), item->IntVar(NAME_MaxAmount));
		count++;
	}
	Printf ("  List count: %d\n", count);
}

