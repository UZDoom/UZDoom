/*
** weaponrlaunch.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
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

// --------------------------------------------------------------------------
//
// Rocket launcher
//
// --------------------------------------------------------------------------

class RocketLauncher : DoomWeapon
{
	Default
	{
		Weapon.SelectionOrder 2500;
		Weapon.AmmoUse 1;
		Weapon.AmmoGive 2;
		Weapon.AmmoType "RocketAmmo";
		+WEAPON.NOAUTOFIRE
		+WEAPON.EXPLOSIVE
		Inventory.PickupMessage "$GOTLAUNCHER";
		Tag "$TAG_ROCKETLAUNCHER";
	}
	States
	{
	Ready:
		MISG A 1 A_WeaponReady;
		Loop;
	Deselect:
		MISG A 1 A_Lower;
		Loop;
	Select:
		MISG A 1 A_Raise;
		Loop;
	Fire:
		MISG B 8 A_GunFlash;
		MISG B 12 A_FireMissile;
		MISG B 0 A_ReFire;
		Goto Ready;
	Flash:
		MISF A 3 Bright A_Light1;
		MISF B 4 Bright;
		MISF CD 4 Bright A_Light2;
		Goto LightDone;
	Spawn:
		LAUN A -1;
		Stop;
	}
}

class Rocket : Actor
{
	Default
	{
		Radius 11;
		Height 8;
		Speed 20;
		Damage 20;
		Projectile;
		+RANDOMIZE
		+DEHEXPLOSION
		+ROCKETTRAIL
		+ZDOOMTRANS
		SeeSound "weapons/rocklf";
		DeathSound "weapons/rocklx";
		Obituary "$OB_MPROCKET";
	}
	States
	{
	Spawn:
		MISL A 1 Bright;
		Loop;
	Death:
		MISL B 8 Bright A_Explode;
		MISL C 6 Bright;
		MISL D 4 Bright;
		Stop;
	BrainExplode:
		MISL BC 10 Bright;
		MISL D 10 A_BrainExplode;
		Stop;
	}
}

// --------------------------------------------------------------------------
//
// Grenade -- Taken and adapted from Skulltag, with MBF stuff added to it
//
// --------------------------------------------------------------------------

class Grenade : Actor
{
	Default
	{
		Radius 8;
		Height 8;
		Speed 25;
		Damage 20;
		Projectile;
		-NOGRAVITY
		+RANDOMIZE
		+DEHEXPLOSION
		+GRENADETRAIL
		BounceType "Doom";
		Gravity 0.25;
		SeeSound "weapons/grenlf";
		DeathSound "weapons/grenlx";
		BounceSound "weapons/grbnce";
		Obituary "$OB_GRENADE";
		DamageType "Grenade";
	}
	States
	{
	Spawn:
		SGRN A 1 Bright;
		Loop;
	Death:
		MISL B 8 Bright A_Explode;
		MISL C 6 Bright;
		MISL D 4 Bright;
		Stop;
	Grenade:
		MISL A 1000 A_Die;
		Wait;
	Detonate:
		MISL B 4 A_Scream;
		MISL C 6 A_Detonate;
		MISL D 10;
		Stop;
	Mushroom:
		MISL B 8 A_Mushroom;
		Goto Death+1;
	}
}

//===========================================================================
//
// Code (must be attached to StateProvider)
//
//===========================================================================

extend class StateProvider
{

	//===========================================================================
	//
	// A_FireMissile
	//
	//===========================================================================

	action void A_FireMissile()
	{
		if (player == null)
		{
			return;
		}
		Weapon weap = player.ReadyWeapon;
		if (weap != null && invoker == weap && stateinfo != null && stateinfo.mStateType == STATE_Psprite)
		{
			if (!weap.DepleteAmmo (weap.bAltFire, true))
				return;
		}
		
		SpawnPlayerMissile ("Rocket");
	}

	//===========================================================================
	//
	// A_FireSTGrenade: not exactly backported from ST, but should work the same
	//
	//===========================================================================

	action void A_FireSTGrenade(class<Actor> grenadetype = "Grenade")
	{
		if (grenadetype == null)
			return;

		if (player == null)
		{
			return;
		}
		Weapon weap = player.ReadyWeapon;
		if (weap != null && invoker == weap && stateinfo != null && stateinfo.mStateType == STATE_Psprite)
		{
			if (!weap.DepleteAmmo (weap.bAltFire, true))
				return;
		}
			
		// Temporarily raise the pitch to send the grenadetype slightly upwards
		double savedpitch = pitch;
		pitch -= 6.328125;
		SpawnPlayerMissile(grenadetype);
		pitch = SavedPitch;
	}
}
