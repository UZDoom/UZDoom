/*
** hereticartifacts.zs
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

// Super map ----------------------------------------------------------------

Class SuperMap : MapRevealer
{
	Default
	{
		+COUNTITEM
		+INVENTORY.ALWAYSPICKUP
		+FLOATBOB
		Inventory.MaxAmount 0;
		Inventory.PickupMessage "$TXT_ITEMSUPERMAP";
		Tag "$TAG_ITEMSUPERMAP";
	}
	States
	{
	Spawn:
		SPMP A -1;
		Stop;
	}
}


// Invisibility -------------------------------------------------------------

Class ArtiInvisibility : PowerupGiver
{
	Default
	{
		+COUNTITEM
		+FLOATBOB
		Inventory.PickupFlash "PickupFlash";
		RenderStyle "Translucent";
		Alpha 0.4;
		Inventory.RespawnTics 4230;
		Inventory.Icon "ARTIINVS";
		Powerup.Type "PowerGhost";
		Inventory.PickupMessage "$TXT_ARTIINVISIBILITY";
		Tag "$TAG_ARTIINVISIBILITY";
	}
	States
	{
	Spawn:
		INVS A 350 Bright;
		Loop;
	}
}

 
// Tome of power ------------------------------------------------------------

Class ArtiTomeOfPower : PowerupGiver
{
	Default
	{
		+COUNTITEM
		+FLOATBOB
		+WEAPONSPAWN
		Inventory.PickupFlash "PickupFlash";
		Inventory.Icon "ARTIPWBK";
		Powerup.Type "PowerWeaponlevel2";
		Inventory.PickupMessage "$TXT_ARTITOMEOFPOWER";
		Tag "$TAG_ARTITOMEOFPOWER";
	}
	States
	{
	Spawn:
		PWBK A 350;
		Loop;
	}
	
	override bool Use(bool pickup)
	{
		EMorphFlags mStyle = Owner.GetMorphStyle();
		if (Owner.Alternative && (mStyle & MRF_UNDOBYTOMEOFPOWER))
		{
			// Attempt to undo chicken.
			if (!Owner.Unmorph(Owner, MRF_UNDOBYTOMEOFPOWER))
			{
				if (!(mStyle & MRF_FAILNOTELEFRAG))
					Owner.DamageMobj(null, null, TELEFRAG_DAMAGE, 'Telefrag');
			}
			else if (Owner.player)
			{
				Owner.A_StartSound("*evillaugh", CHAN_VOICE);
			}

			return true;
		}

		return Super.Use(pickup);
	}
	
}


// Time bomb ----------------------------------------------------------------

Class ActivatedTimeBomb : Actor
{
	Default
	{
		+NOGRAVITY
		RenderStyle "Translucent";
		Alpha 0.4;
		DeathSound "misc/timebomb";
	}
		
	States
	{
	Spawn:
		FBMB ABCD 10;
		FBMB E 6 A_Scream;
		XPL1 A 4 BRIGHT A_Timebomb;
		XPL1 BCDEF 4 BRIGHT;
		Stop;
	}

	void A_TimeBomb()
	{
		AddZ(32, false);
		A_SetRenderStyle(1., STYLE_Add);
		A_Explode();
	}
}


Class ArtiTimeBomb : Inventory
{
	Default
	{
		+COUNTITEM
		+FLOATBOB
		+WEAPONSPAWN
		Inventory.PickupFlash "PickupFlash";
		+INVENTORY.INVBAR
		+INVENTORY.FANCYPICKUPSOUND
		Inventory.Icon "ARTIFBMB";
		Inventory.PickupSound "misc/p_pkup";
		Inventory.PickupMessage "$TXT_ARTIFIREBOMB";
		Tag "$TAG_ARTIFIREBOMB";
		Inventory.DefMaxAmount;
	}
	States
	{
	Spawn:
		FBMB E 350;
		Loop;
	}
	
	override bool Use (bool pickup)
	{
		Actor mo = Spawn("ActivatedTimeBomb", Owner.Vec3Angle(24., Owner.angle, - Owner.Floorclip), ALLOW_REPLACE);
		if (mo != null) mo.target = Owner;
		return true;
	}
	
}
