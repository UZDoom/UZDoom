/*
** doomweapons.zs
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
// Doom weap base class
//
// --------------------------------------------------------------------------

class DoomWeapon : Weapon
{
	Default
	{
		Weapon.Kickback 100;
	}
}



extend class StateProvider
{

	//
	// [RH] A_FireRailgun
	// [TP] This now takes a puff type to retain Skulltag's railgun's ability to pierce armor.
	//
	action void A_FireRailgun(class<Actor> puffType = "BulletPuff", int offset_xy = 0)
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
				player.SetSafeFlash(weap, flash, random[FireRail](0, 1));
			}
			
		}

		int damage = deathmatch ? 100 : 150;
		A_RailAttack(damage, offset_xy, false, pufftype: puffType);	// note that this function handles ammo depletion itself for Dehacked compatibility purposes.
	}

	action void A_FireRailgunLeft()
	{
		A_FireRailgun(offset_xy: -10);
	}

	action void A_FireRailgunRight()
	{
		A_FireRailgun(offset_xy: 10);
	}

	action void A_RailWait() 
	{
		// only here to satisfy old Dehacked patches.
	}

}
