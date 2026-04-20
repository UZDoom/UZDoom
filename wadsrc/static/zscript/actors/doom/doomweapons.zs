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
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
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
