/*
** a_pickups.h
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

#ifndef __A_PICKUPS_H__
#define __A_PICKUPS_H__

#include "actor.h"
#include "info.h"
#include "s_sound.h"

#define NUM_WEAPON_SLOTS		10

class player_t;
class FConfigFile;

// This encapsulates the fields of vissprite_t that can be altered by AlterWeaponSprite
struct visstyle_t
{
	bool			Invert;
	float			Alpha;
	ERenderStyle	RenderStyle;
};



/************************************************************************/
/* Class definitions													*/
/************************************************************************/

// A pickup is anything the player can pickup (i.e. weapons, ammo, powerups, etc)

bool CallTryPickup(AActor *item, AActor *toucher, AActor **toucher_return = nullptr);
void DepleteOrDestroy(AActor *item);			// virtual on the script side. 

#endif //__A_PICKUPS_H__
