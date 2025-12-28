/*
** weaponplasma.zs
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
// Plasma rifle
//
// --------------------------------------------------------------------------

class PlasmaRifle : DoomWeapon
{
	Default
	{
		Weapon.SelectionOrder 100;
		Weapon.AmmoUse 1;
		Weapon.AmmoGive 40;
		Weapon.AmmoType "Cell";
		Inventory.PickupMessage "$GOTPLASMA";
		Tag "$TAG_PLASMARIFLE";
	}
	States
	{
	Ready:
		PLSG A 1 A_WeaponReady;
		Loop;
	Deselect:
		PLSG A 1 A_Lower;
		Loop;
	Select:
		PLSG A 1 A_Raise;
		Loop;
	Fire:
		PLSG A 3 A_FirePlasma;
		PLSG B 20 A_ReFire;
		Goto Ready;
	Flash:
		PLSF A 4 Bright A_Light1;
		Goto LightDone;
		PLSF B 4 Bright A_Light1;
		Goto LightDone;
	Spawn:
		PLAS A -1;
		Stop;
	}
}

class PlasmaBall : Actor
{
	Default
	{
		Radius 13;
		Height 8;
		Speed 25;
		Damage 5;
		Projectile;
		+RANDOMIZE
		+ZDOOMTRANS
		RenderStyle "Add";
		Alpha 0.75;
		SeeSound "weapons/plasmaf";
		DeathSound "weapons/plasmax";
		Obituary "$OB_MPPLASMARIFLE";
	}
	States
	{
	Spawn:
		PLSS AB 6 Bright;
		Loop;
	Death:
		PLSE ABCDE 4 Bright;
		Stop;
	}
}

// --------------------------------------------------------------------------
//
// BFG 2704
//
// --------------------------------------------------------------------------

class PlasmaBall1 : PlasmaBall
{
	Default
	{
		Damage 4;
		BounceType "Classic";
		BounceFactor 1.0;
		Obituary "$OB_MPBFG_MBF";
	}
	States
	{
	Spawn:
		PLS1 AB 6 Bright;
		Loop;
	Death:
		PLS1 CDEFG 4 Bright;
		Stop;
	}
}
	
class PlasmaBall2 : PlasmaBall1
{
	States
	{
	Spawn:
		PLS2 AB 6 Bright;
		Loop;
	Death:
		PLS2 CDE 4 Bright;
		Stop;
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
	// A_FirePlasma
	//
	//===========================================================================

	action void A_FirePlasma()
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
			
			State flash = weap.FindState('Flash');
			if (flash != null)
			{
				player.SetSafeFlash(weap, flash, random[FirePlasma](0, 1));
			}
			
		}
		
		SpawnPlayerMissile ("PlasmaBall");
	}
}
