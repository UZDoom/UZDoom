/*
** weaponpiece.zs
**
** Implements generic weapon pieces
**
**---------------------------------------------------------------------------
**
** Copyright 2006-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

class WeaponHolder : Inventory
{
	int PieceMask;
	Class<Weapon> PieceWeapon;
	
	Default
	{
		+NOBLOCKMAP
		+NOSECTOR
		+INVENTORY.UNDROPPABLE
	}
}

class WeaponPiece : Inventory
{
	Default
	{
		+WEAPONSPAWN;
	}
	
	int PieceValue;
	Class<Weapon> WeaponClass;
	Weapon FullWeapon;
	
	property number: PieceValue;
	property weapon: WeaponClass;

	// Account for weapon replacers, but make sure it's still a Weapon
	clearscope class<Weapon> GetWeaponClass() const
	{
		class<Weapon> type = WeaponClass ? (class<Weapon>)(GetReplacement(WeaponClass)) : null;
		return type ? type : WeaponClass;
	}
	
	//==========================================================================
	//
	// TryPickupWeaponPiece
	//
	//==========================================================================

	override bool TryPickupRestricted (in out Actor toucher)
	{
		// Wrong class, but try to pick up for ammo
		if (ShouldStay())
		{ // Can't pick up weapons for other classes in coop netplay
			return false;
		}

		class<Weapon> type = GetWeaponClass();
		if (!type)
			return false;

		let Defaults = GetDefaultByType(type);

		bool gaveSome = !!(toucher.GiveAmmo (Defaults.AmmoType1, Defaults.AmmoGive1) +
						   toucher.GiveAmmo (Defaults.AmmoType2, Defaults.AmmoGive2));

		if (gaveSome)
		{
			GoAwayAndDie ();
		}
		return gaveSome;
	}

	//==========================================================================
	//
	// TryPickupWeaponPiece
	//
	//==========================================================================

	override bool TryPickup (in out Actor toucher)
	{
		class<Weapon> type = GetWeaponClass();
		if (!type)
			return false;

		Inventory item;
		WeaponHolder hold = NULL;
		bool shouldStay = ShouldStay ();
		int gaveAmmo;
		let Defaults = GetDefaultByType(type);

		FullWeapon = NULL;
		for(item=toucher.Inv; item; item=item.Inv)
		{
			hold = WeaponHolder(item);
			if (hold != null)
			{
				// Intentionally check against the unreplaced class
				if (hold.PieceWeapon == WeaponClass) 
				{
					break;
				}
				hold = NULL;
			}
		}
		if (!hold)
		{
			hold = WeaponHolder(Spawn("WeaponHolder"));
			hold.BecomeItem();
			hold.AttachToOwner(toucher);
			hold.PieceMask = 0;
			hold.PieceWeapon = WeaponClass;
		}

		int pieceval = 1 << (PieceValue - 1);
		if (shouldStay)
		{ 
			// Cooperative net-game
			if (hold.PieceMask & pieceval)
			{ 
				// Already has the piece
				return false;
			}
			toucher.GiveAmmo (Defaults.AmmoType1, Defaults.AmmoGive1);
			toucher.GiveAmmo (Defaults.AmmoType2, Defaults.AmmoGive2);
		}
		else
		{ // Deathmatch or singleplayer game
			gaveAmmo = toucher.GiveAmmo (Defaults.AmmoType1, Defaults.AmmoGive1) +
						toucher.GiveAmmo (Defaults.AmmoType2, Defaults.AmmoGive2);
			
			if (hold.PieceMask & pieceval)
			{ 
				// Already has the piece, check if mana needed
				if (!gaveAmmo) return false;
				GoAwayAndDie();
				return true;
			}
		}

		hold.PieceMask |= pieceval;

		// Check if  weapon assembled
		if (hold.PieceMask == (1 << Defaults.health) - 1)
		{
			if (!toucher.FindInventory (type))
			{
				FullWeapon= Weapon(Spawn(type));
				
				// The weapon itself should not give more ammo to the player.
				FullWeapon.AmmoGive1 = 0;
				FullWeapon.AmmoGive2 = 0;
				FullWeapon.AttachToOwner(toucher);
				FullWeapon.AmmoGive1 = Defaults.AmmoGive1;
				FullWeapon.AmmoGive2 = Defaults.AmmoGive2;
			}
		}
		GoAwayAndDie();
		return true;
	}

	//===========================================================================
	//
	//
	//
	//===========================================================================

	override bool ShouldStay ()
	{
		// We want a weapon piece to behave like a weapon, so follow the exact
		// same logic as weapons when deciding whether or not to stay.
		return (((multiplayer &&
			(!deathmatch && !alwaysapplydmflags)) || sv_weaponstay) && !bDropped);
	}

	//===========================================================================
	//
	// PickupMessage
	//
	// Returns the message to print when this actor is picked up.
	//
	//===========================================================================

	override String PickupMessage ()
	{
		if (FullWeapon) 
		{
			return FullWeapon.PickupMessage();
		}
		else
		{
			return Super.PickupMessage();
		}
	}

	//===========================================================================
	//
	// DoPlayPickupSound
	//
	// Plays a sound when this actor is picked up.
	//
	//===========================================================================

	override void PlayPickupSound (Actor toucher)
	{
		if (FullWeapon)
		{
			FullWeapon.PlayPickupSound(toucher);
		}
		else
		{
			Super.PlayPickupSound(toucher);
		}
	}
}
