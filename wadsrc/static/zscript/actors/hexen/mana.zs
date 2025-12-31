/*
** mana.zs
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

// Blue mana ----------------------------------------------------------------

class Mana1 : Ammo
{
	Default
	{
		Inventory.Amount 15;
		Inventory.MaxAmount 200;
		Ammo.BackpackAmount 15;
		Ammo.BackpackMaxAmount 200;
		Radius 8;
		Height 8;
		+FLOATBOB
		Inventory.Icon "MAN1I0";
		Inventory.PickupMessage "$TXT_MANA_1";
		Tag "$AMMO_MANA1";
	}
	States
	{
	Spawn:
		MAN1 ABCDEFGHI 4 Bright;
		Loop;
	}
}

// Green mana ---------------------------------------------------------------

class Mana2 : Ammo
{
	Default
	{
		Inventory.Amount 15;
		Inventory.MaxAmount 200;
		Ammo.BackpackAmount 15;
		Ammo.BackpackMaxAmount 200;
		Radius 8;
		Height 8;
		+FLOATBOB
		Inventory.Icon "MAN2G0";
		Inventory.PickupMessage "$TXT_MANA_2";
		Tag "$AMMO_MANA2";
	}
	States
	{
	Spawn:
		MAN2 ABCDEFGHIJKLMNOP 4 Bright;
		Loop;
	}
}

// Combined mana ------------------------------------------------------------

class Mana3 : CustomInventory
{
	Default
	{
		Radius 8;
		Height 8;
		+FLOATBOB
		+WEAPONSPAWN
		Inventory.PickupMessage "$TXT_MANA_BOTH";
	}
	States
	{
	Spawn:
		MAN3 ABCDEFGHIJKLMNOP 4 Bright;
		Loop;
	Pickup:
		TNT1 A 0 A_GiveInventory("Mana1", 20);
		TNT1 A 0 A_GiveInventory("Mana2", 20);
		Stop;
	}
}

// Boost Mana Artifact Krater of Might ------------------------------------

class ArtiBoostMana : CustomInventory
{
	Default
	{
		+FLOATBOB
		+COUNTITEM
		+WEAPONSPAWN
		+INVENTORY.INVBAR
		+INVENTORY.FANCYPICKUPSOUND
		Inventory.PickupFlash "PickupFlash";
		Inventory.DefMaxAmount;
		Inventory.Icon "ARTIBMAN";
		Inventory.PickupSound "misc/p_pkup";
		Inventory.PickupMessage "$TXT_ARTIBOOSTMANA";
		Tag "$TAG_ARTIBOOSTMANA";
	}
	States
	{
	Spawn:
		BMAN A -1;
		Stop;
	Use:
		TNT1 A 0 A_GiveInventory("Mana1", 200);
		TNT1 A 0 A_GiveInventory("Mana2", 200);
		Stop;
	}
}
