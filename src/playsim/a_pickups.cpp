/*
** a_pickups.cpp
**
** Inventory base class implementation
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
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

#include <assert.h>

#include "gstrings.h"
#include "sbar.h"
#include "doomstat.h"
#include "d_player.h"
#include "vm.h"
#include "g_levellocals.h"
#include "gi.h"

EXTERN_CVAR(Bool, sv_unlimited_pickup)


//===========================================================================
//
// This is only native so it can have some static storage for comparison.
//
//===========================================================================
static int StaticLastMessageTic;
static FString StaticLastMessage;

void PrintPickupMessage(bool localview, const FString &str)
{
	// [MK] merge identical messages on same tic unless disabled in gameinfo
	if (str.IsNotEmpty() && localview && (gameinfo.nomergepickupmsg || StaticLastMessageTic != gametic || StaticLastMessage.Compare(str)))
	{
		StaticLastMessageTic = gametic;
		StaticLastMessage = str;
		const char *pstr = str.GetChars();
		
		if (pstr[0] == '$')	pstr = GStrings.GetString(pstr + 1);
		if (pstr[0] != 0) Printf(PRINT_LOW, "%s\n", pstr);
		StatusBar->FlashCrosshair();
	}
}

//===========================================================================
//
// AInventory :: DepleteOrDestroy
//
// If the item is depleted, just change its amount to 0, otherwise it's destroyed.
//
//===========================================================================

void DepleteOrDestroy (AActor *item)
{
	IFVIRTUALPTRNAME(item, NAME_Inventory, DepleteOrDestroy)
	{
		VMValue params[1] = { item };
		VMCall(func, params, 1, nullptr, 0);
	}
}

//===========================================================================
//
// AInventory :: CallTryPickup
//
//===========================================================================

bool CallTryPickup(AActor *item, AActor *toucher, AActor **toucher_return)
{
	static VMFunction *func = nullptr;
	if (func == nullptr) PClass::FindFunction(&func, NAME_Inventory, NAME_CallTryPickup);
	VMValue params[2] = { (DObject*)item, toucher };
	VMReturn ret[2];
	int res;
	AActor *tret;
	ret[0].IntAt(&res);
	ret[1].PointerAt((void**)&tret);
	VMCall(func, params, 2, ret, 2);
	if (toucher_return) *toucher_return = tret;
	return !!res;
}

