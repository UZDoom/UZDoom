/*
** strifearmor.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Rogue Entertainment
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

class MetalArmor : BasicArmorPickup
{
	Default
	{
		Radius 20;
		Height 16;
		+FLOORCLIP 
		+INVENTORY.AUTOACTIVATE 
		+INVENTORY.INVBAR
		Inventory.MaxAmount 3;
		Inventory.Icon "I_ARM1";
		Inventory.PickupMessage "$TXT_METALARMOR";
		Armor.SaveAmount 200;
		Armor.SavePercent 50;
		Tag "$TAG_METALARMOR";
	}
	States
	{
	Spawn:
		ARM3 A -1;
		Stop;
	}
}

class LeatherArmor : BasicArmorPickup
{
	Default
	{
		Radius 20;
		Height 16;
		+FLOORCLIP 
		+INVENTORY.AUTOACTIVATE 
		+INVENTORY.INVBAR
		Inventory.MaxAmount 5;
		Inventory.Icon "I_ARM2";
		Inventory.PickupMessage "$TXT_LEATHERARMOR";
		Armor.SaveAmount 100;
		Armor.SavePercent 33.335;
		Tag "$TAG_LEATHER";
	}
	States
	{
	Spawn:
		ARM4 A -1;
		Stop;
	}
}

