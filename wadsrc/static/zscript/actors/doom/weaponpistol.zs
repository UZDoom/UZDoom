/*
** weaponpistol.zs
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
// Pistol 
//
// --------------------------------------------------------------------------

class Pistol : DoomWeapon
{
	Default
	{
		Weapon.SelectionOrder 1900;
		Weapon.AmmoUse 1;
		Weapon.AmmoGive 20;
		Weapon.AmmoType "Clip";
		Obituary "$OB_MPPISTOL";
		+WEAPON.WIMPY_WEAPON
		Inventory.Pickupmessage "$PICKUP_PISTOL_DROPPED";
		Tag "$TAG_PISTOL";
	}
	States
	{
	Ready:
		PISG A 1 A_WeaponReady;
		Loop;
	Deselect:
		PISG A 1 A_Lower;
		Loop;
	Select:
		PISG A 1 A_Raise;
		Loop;
	Fire:
		PISG A 4;
		PISG B 6 A_FirePistol;
		PISG C 4;
		PISG B 5 A_ReFire;
		Goto Ready;
	Flash:
		PISF A 7 Bright A_Light1;
		Goto LightDone;
		PISF A 7 Bright A_Light1;
		Goto LightDone;
 	Spawn:
		PIST A -1;
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
	// This is also used by the shotgun and chaingun
	//===========================================================================
	
	protected action void GunShot(bool accurate, Class<Actor> pufftype, double pitch)
	{
		int damage = 5 * random[GunShot](1, 3);
		double ang = angle;

		if (!accurate)
		{
			ang += Random2[GunShot]() * (5.625 / 256);

			if (GetCVar ("vertspread") && !sv_novertspread)
			{
				pitch += Random2[GunShot]() * (3.549 / 256);
			}
		}

		LineAttack(ang, PLAYERMISSILERANGE, pitch, damage, 'Hitscan', pufftype);
	}
	
	//===========================================================================
	action void A_FirePistol()
	{
		bool accurate;

		if (player != null)
		{
			Weapon weap = player.ReadyWeapon;
			if (weap != null && invoker == weap && stateinfo != null && stateinfo.mStateType == STATE_Psprite)
			{
				if (!weap.DepleteAmmo (weap.bAltFire, true))
					return;

				player.SetPsprite(PSP_FLASH, weap.FindState('Flash'), true);
			}
			player.mo.PlayAttacking2 ();

			accurate = !player.refire;
		}
		else
		{
			accurate = true;
		}

		A_StartSound ("weapons/pistol", CHAN_WEAPON);
		GunShot (accurate, "BulletPuff", BulletSlope ());
	}
}
