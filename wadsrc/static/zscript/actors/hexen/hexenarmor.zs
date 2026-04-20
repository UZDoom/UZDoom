/*
** hexenarmor.zs
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

// Mesh Armor (1) -----------------------------------------------------------

class MeshArmor : HexenArmor
{
	Default
	{
		+NOGRAVITY
		Health 0;	// Armor class
		Inventory.Amount 0;
		Inventory.PickupMessage "$TXT_ARMOR1";
	}
	States
	{
	Spawn:
		AR_1 A -1;
		Stop;
	}
}
	
// Falcon Shield (2) --------------------------------------------------------

class FalconShield : HexenArmor
{
	Default
	{
		+NOGRAVITY
		Health 1;	// Armor class
		Inventory.Amount 0;
		Inventory.PickupMessage "$TXT_ARMOR2";
	}
	States
	{
	Spawn:
		AR_2 A -1;
		Stop;
	}
}

// Platinum Helm (3) --------------------------------------------------------

class PlatinumHelm : HexenArmor
{
	Default
	{
		+NOGRAVITY
		Health 2;	// Armor class
		Inventory.Amount 0;
		Inventory.PickupMessage "$TXT_ARMOR3";
	}
	States
	{
	Spawn:
		AR_3 A -1;
		Stop;
	}
}

// Amulet of Warding (4) ----------------------------------------------------

class AmuletOfWarding : HexenArmor
{
	Default
	{
		+NOGRAVITY
		Health 3;	// Armor class
		Inventory.Amount 0;
		Inventory.PickupMessage "$TXT_ARMOR4";
	}
	States
	{
	Spawn:
		AR_4 A -1;
		Stop;
	}
}

