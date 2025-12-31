/*
** healingradius.zs
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

// Healing Radius Artifact --------------------------------------------------

class ArtiHealingRadius : Inventory
{
	const HEAL_RADIUS_DIST = 255.;

	Default
	{
		+COUNTITEM
		+FLOATBOB
		Inventory.DefMaxAmount;
		+INVENTORY.INVBAR 
		+INVENTORY.FANCYPICKUPSOUND
		Inventory.PickupFlash "PickupFlash";
		Inventory.Icon "ARTIHRAD";
		Inventory.PickupSound "misc/p_pkup";
		Inventory.PickupMessage "$TXT_ARTIHEALINGRADIUS";
		Tag "$TAG_ARTIHEALINGRADIUS";
	}
	States
	{
	Spawn:
		HRAD ABCDEFGHIJKLMNOP 4 Bright;
		Loop;
	}	
	
	override bool Use (bool pickup)
	{
		bool effective = false;
		Name mode = 'Health';
		
		PlayerPawn pp = PlayerPawn(Owner);
		if (pp) mode = pp.HealingRadiusType;

		for (int i = 0; i < MAXPLAYERS; ++i)
		{
			if (!playeringame[i])
			{
				continue;
			}

			PlayerPawn mo = players[i].mo;
			if (mo != null && mo.health > 0 && mo.Distance2D (Owner) <= HEAL_RADIUS_DIST)
			{
				// Q: Is it worth it to make this selectable as a player property?
				// A: Probably not - but it sure doesn't hurt.
				bool gotsome=false;
				switch (mode)
				{
				case 'Armor':
					for (int j = 0; j < 4; ++j)
					{
						HexenArmor armor = HexenArmor(Spawn(GetHexenArmorClass()));
						armor.health = j;
						armor.Amount = 1;
						if (!armor.CallTryPickup (mo))
						{
							armor.Destroy ();
						}
						else
						{
							gotsome = true;
						}
					}
					break;

				case 'Mana':
				{
					int amount = random[HealRadius](50, 99);

					if (mo.GiveAmmo ("Mana1", amount) ||
						mo.GiveAmmo ("Mana2", amount))
					{
						gotsome = true;
					}
					break;
				}

				default:
				//case NAME_Health:
					gotsome = mo.GiveBody (random[HealRadius](50, 99));
					break;
				}
				if (gotsome)
				{
					mo.A_StartSound ("MysticIncant", CHAN_AUTO);
					effective=true;
				}
			}
		}
		return effective;

	}
	
}

