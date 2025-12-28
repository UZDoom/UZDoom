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
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

