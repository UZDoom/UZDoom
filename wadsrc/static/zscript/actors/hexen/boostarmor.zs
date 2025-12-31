/*
** boostarmor.zs
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

// Boost Armor Artifact (Dragonskin Bracers) --------------------------------

class ArtiBoostArmor : Inventory
{
	Default
	{
		+COUNTITEM
		+FLOATBOB
		Inventory.DefMaxAmount;
		Inventory.PickupFlash "PickupFlash";
		+INVENTORY.INVBAR +INVENTORY.FANCYPICKUPSOUND
		Inventory.Icon "ARTIBRAC";
		Inventory.PickupSound "misc/p_pkup";
		Inventory.PickupMessage "$TXT_ARTIBOOSTARMOR";
		Tag "$TAG_ARTIBOOSTARMOR";
	}
	States
	{
	Spawn:
		BRAC ABCDEFGH 4 Bright;
		Loop;
	}
	
	override bool Use (bool pickup)
	{
		int count = 0;

		if (gameinfo.gametype == GAME_Hexen)
		{
			HexenArmor armor;

			for (int i = 0; i < 4; ++i)
			{
				armor = HexenArmor(Spawn(GetHexenArmorClass()));
				armor.bDropped = true;
				armor.health = i;
				armor.Amount = 1;
				if (!armor.CallTryPickup (Owner))
				{
					armor.Destroy ();
				}
				else
				{
					count++;
				}
			}
			return count != 0;
		}
		else
		{
			BasicArmorBonus armor = BasicArmorBonus(Spawn("BasicArmorBonus"));
			armor.bDropped = true;
			armor.SaveAmount = 50;
			armor.MaxSaveAmount = 300;
			if (!armor.CallTryPickup (Owner))
			{
				armor.Destroy ();
				return false;
			}
			else
			{
				return true;
			}
		}
	}

	
}
