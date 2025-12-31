/*
** doomarmor.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
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

// Armor bonus --------------------------------------------------------------

class ArmorBonus : BasicArmorBonus
{
	Default
	{
		Radius 20;
		Height 16;
		Inventory.Pickupmessage "$GOTARMBONUS";
		Inventory.Icon "BON2A0";
		Armor.Savepercent 33.335;
		Armor.Saveamount 1;
		Armor.Maxsaveamount 200;
		+COUNTITEM
		+INVENTORY.ALWAYSPICKUP
		Tag "$TAG_ARMORBONUS";
	}
	States
	{
	Spawn:
		BON2 ABCDCB 6;
		loop;
	}
}

// Green armor --------------------------------------------------------------

class GreenArmor : BasicArmorPickup
{
	Default
	{
		Radius 20;
		Height 16;
		Inventory.Pickupmessage "$GOTARMOR";
		Inventory.Icon "ARM1A0";
		Armor.SavePercent 33.335;
		Armor.SaveAmount 100;
		Tag "$TAG_GREENARMOR";
	}
	States
	{
	Spawn:
		ARM1 A 6;
		ARM1 B 7 bright;
		loop;
	}
}

// Blue armor ---------------------------------------------------------------

class BlueArmor : BasicArmorPickup
{
	Default
	{
		Radius 20;
		Height 16;
		Inventory.Pickupmessage "$GOTMEGA";
		Inventory.Icon "ARM2A0";
		Armor.Savepercent 50;
		Armor.Saveamount 200;
		Tag "$TAG_BLUEARMOR";
	}
	States
	{
	Spawn:
		ARM2 A 6;
		ARM2 B 6 bright;
		loop;
	}
}

