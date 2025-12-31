/*
** weaponcrossbow.zs
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

// Crossbow -----------------------------------------------------------------

class Crossbow : HereticWeapon
{
	Default
	{
		Weapon.SelectionOrder 800;
		Weapon.AmmoUse 1;
		Weapon.AmmoGive 10;
		Weapon.AmmoType "CrossbowAmmo";
		Weapon.SisterWeapon "CrossbowPowered";
		Weapon.YAdjust 15;
		Inventory.PickupMessage "$TXT_WPNCROSSBOW";
		Tag "$TAG_CROSSBOW";
	}

	States
	{
	Spawn:
		WBOW A -1;
		Stop;
	Ready:
		CRBW AAAAAABBBBBBCCCCCC 1 A_WeaponReady;
		Loop;
	Deselect:
		CRBW A 1 A_Lower;
		Loop;
	Select:
		CRBW A 1 A_Raise;
		Loop;
	Fire:
		CRBW D 6 A_FireCrossbowPL1;
		CRBW EFGH 3;
		CRBW AB 4;
		CRBW C 5 A_ReFire;
		Goto Ready;
	}
	
	//----------------------------------------------------------------------------
	//
	// PROC A_FireCrossbowPL1
	//
	//----------------------------------------------------------------------------

	action void A_FireCrossbowPL1 ()
	{
		if (player == null)
		{
			return;
		}

		Weapon weapon = player.ReadyWeapon;
		if (weapon != null)
		{
			if (!weapon.DepleteAmmo (weapon.bAltFire))
				return;
		}
		SpawnPlayerMissile ("CrossbowFX1");
		SpawnPlayerMissile ("CrossbowFX3", angle - 4.5);
		SpawnPlayerMissile ("CrossbowFX3", angle + 4.5);
	}
}


class CrossbowPowered : Crossbow
{
	Default
	{
		+WEAPON.POWERED_UP
		Weapon.AmmoGive 0;
		Weapon.SisterWeapon "Crossbow";
		Tag "$TAG_CROSSBOWP";
	}

	States
	{
	Fire:
		CRBW D 5 A_FireCrossbowPL2;
		CRBW E 3;
		CRBW F 2;
		CRBW G 3;
		CRBW H 2;
		CRBW A 3;
		CRBW B 3;
		CRBW C 4 A_ReFire;
		Goto Ready;
	}
	
	//----------------------------------------------------------------------------
	//
	// PROC A_FireCrossbowPL2
	//
	//----------------------------------------------------------------------------

	action void A_FireCrossbowPL2()
	{
		if (player == null)
		{
			return;
		}

		Weapon weapon = player.ReadyWeapon;
		if (weapon != null)
		{
			if (!weapon.DepleteAmmo (weapon.bAltFire))
				return;
		}
		SpawnPlayerMissile ("CrossbowFX2");
		SpawnPlayerMissile ("CrossbowFX2", angle - 4.5);
		SpawnPlayerMissile ("CrossbowFX2", angle + 4.5);
		SpawnPlayerMissile ("CrossbowFX3", angle - 9.);
		SpawnPlayerMissile ("CrossbowFX3", angle + 9.);
	}
}


// Crossbow FX1 -------------------------------------------------------------

class CrossbowFX1 : Actor
{
	Default
	{
		Radius 11;
		Height 8;
		Speed 30;
		Damage 10;
		Projectile;
		RenderStyle "Add";
		SeeSound "weapons/bowshoot";
		DeathSound "weapons/bowhit";
		Obituary "$OB_MPCROSSBOW";
		+ZDOOMTRANS
	}

	States
	{
	Spawn:
		FX03 B 1 BRIGHT;
		Loop;
	Death:
		FX03 HIJ 8 BRIGHT;
		Stop;
	}
}


// Crossbow FX2 -------------------------------------------------------------

class CrossbowFX2 : CrossbowFX1
{
	Default
	{
		Speed 32;
		Damage 6;
		Obituary "$OB_MPPCROSSBOW";
	}

	States
	{
	Spawn:
		FX03 B 1 BRIGHT A_SpawnItemEx("CrossbowFX4", random2[BoltSpark]()*0.015625, random2[BoltSpark]()*0.015625, 0, 0,0,0,0,SXF_ABSOLUTEPOSITION, 50);
		Loop;
	}
}

// Crossbow FX3 -------------------------------------------------------------

class CrossbowFX3 : CrossbowFX1
{
	Default
	{
		Speed 20;
		Damage 2;
		SeeSound "";
		-NOBLOCKMAP
		+WINDTHRUST
		+THRUGHOST
	}

	States
	{
	Spawn:
		FX03 A 1 BRIGHT;
		Loop;
	Death:
		FX03 CDE 8 BRIGHT;
		Stop;
	}
}

// Crossbow FX4 -------------------------------------------------------------

class CrossbowFX4 : Actor
{
	Default
	{
		+NOBLOCKMAP
		Gravity 0.125;
		RenderStyle "Add";
	}

	States
	{
	Spawn:
		FX03 FG 8 BRIGHT;
		Stop;
	}
}

