/*
** weaponmissile.zs
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

// Mini-Missile Launcher ----------------------------------------------------

class MiniMissileLauncher : StrifeWeapon
{
	Default
	{
		+FLOORCLIP
		Weapon.SelectionOrder 1800;
		Weapon.AmmoUse1 1;
		Weapon.AmmoGive1 8;
		Weapon.AmmoType1 "MiniMissiles";
		Inventory.Icon "MMSLA0";
		Tag "$TAG_MMLAUNCHER";
		Inventory.PickupMessage "$TXT_MMLAUNCHER";
		+WEAPON.EXPLOSIVE
	}

	States
	{
	Spawn:
		MMSL A -1;
		Stop;
	Ready:
		MMIS A 1 A_WeaponReady;
		Loop;
	Deselect:
		MMIS A 1 A_Lower;
		Loop;
	Select:
		MMIS A 1 A_Raise;
		Loop;
	Fire:
		MMIS A 4 A_FireMiniMissile;
		MMIS B 4 A_Light1;
		MMIS C 5 Bright;
		MMIS D 2 Bright A_Light2;
		MMIS E 2 Bright;
		MMIS F 2 Bright A_Light0;
		MMIS F 0 A_ReFire;
		Goto Ready;
	}
	
	//============================================================================
	//
	// A_FireMiniMissile
	//
	//============================================================================

	action void A_FireMiniMissile ()
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
		
		double savedangle = angle;
		angle += Random2[MiniMissile]() * (11.25 / 256) * AccuracyFactor();
		player.mo.PlayAttacking2 ();
		SpawnPlayerMissile ("MiniMissile");
		angle = savedangle;
	}
}
	

// Rocket Trail -------------------------------------------------------------

class RocketTrail : Actor
{
	Default
	{
		+NOBLOCKMAP
		+NOGRAVITY
		RenderStyle "Translucent";
		Alpha 0.25;
		SeeSound "misc/missileinflight";
	}
	States
	{
	Spawn:
		PUFY BCBCD 4;
		Stop;
	}
}

// Rocket Puff --------------------------------------------------------------

class MiniMissilePuff : StrifePuff
{
	Default
	{
		-ALLOWPARTICLES
	}
	States
	{
	Spawn:
		Goto Crash;
	}
}

// Mini Missile -------------------------------------------------------------

class MiniMissile : Actor
{
	Default
	{
		Speed 20;
		Radius 10;
		Height 14;
		Damage 10;
		Projectile;
		+STRIFEDAMAGE
		MaxStepHeight 4;
		SeeSound "weapons/minimissile";
		DeathSound "weapons/minimissilehit";
		Obituary "$OB_MPMINIMISSILELAUNCHER";
	}
	States
	{
	Spawn:
		MICR A 6 Bright A_RocketInFlight;
		Loop;
	Death:
		SMIS A 0 Bright A_SetRenderStyle(1, STYLE_Normal);
		SMIS A 5 Bright A_Explode(64, 64, alert:true);
		SMIS B 5 Bright;
		SMIS C 4 Bright;
		SMIS DEFG 2 Bright;
		Stop;
	}
}

