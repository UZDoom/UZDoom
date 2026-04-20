/*
** speedboots.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class ArtiSpeedBoots : PowerupGiver
{
	Default
	{
		+FLOATBOB
		+COUNTITEM
		Inventory.PickupFlash "PickupFlash";
		Inventory.Icon "ARTISPED";
		Inventory.PickupMessage "$TXT_ARTISPEED";
		Tag "$TAG_ARTISPEED";
		Powerup.Type "PowerSpeed";
	}
	States
	{
	Spawn:
		SPED ABCDEFGH 3 Bright;
		Loop;
	}
}
