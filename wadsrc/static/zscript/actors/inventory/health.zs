/*
** health.zs
**
** All health items
**
**---------------------------------------------------------------------------
**
** Copyright 2000-2016 Marisa Heit
** Copyright 2006-2017 Christoph Oelckers
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

class Health : Inventory
{
	transient int PrevHealth;
	meta int LowHealth;
	meta String LowHealthMessage;
	
	property LowMessage: LowHealth, LowHealthMessage;
	
	Default
	{
		+INVENTORY.ISHEALTH
		Inventory.Amount 1;
		Inventory.MaxAmount 0;
		Inventory.PickupSound "misc/health_pkup";
	}
	
	//===========================================================================
	//
	// AHealth :: PickupMessage
	//
	//===========================================================================
	override String PickupMessage ()
	{
		if (PrevHealth < LowHealth)
		{
			String message = LowHealthMessage;
			if (message.Length() != 0)
			{
				return message;
			}
		}
		return Super.PickupMessage();
	}

	//===========================================================================
	//
	// TryPickup
	//
	//===========================================================================

	override bool TryPickup (in out Actor other)
	{
		PrevHealth = other.player != NULL ? other.player.health : other.health;

		// P_GiveBody adds one new feature, applied only if it is possible to pick up negative health:
		// Negative values are treated as positive percentages, ie Amount -100 means 100% health, ignoring max amount.
		if (other.GiveBody(Amount, MaxAmount))
		{
			GoAwayAndDie();
			return true;
		}
		return false;
	}
}

class MaxHealth : Health
{
	//===========================================================================
	//
	// TryPickup
	//
	//===========================================================================

	override bool TryPickup (in out Actor other)
	{
		bool success = false;
		let player = PlayerPawn(other);
		if (player)
		{
			if (player.BonusHealth < MaxAmount)
			{
				player.BonusHealth = min(player.BonusHealth + Amount, MaxAmount);
				success = true;
			}
		}
		success |= Super.TryPickup(other);
		if (success) GoAwayAndDie();
		return success;
	}
}

class HealthPickup : Inventory
{
	int autousemode;

	property AutoUse: autousemode;

	Default
	{
		Inventory.DefMaxAmount;
		+INVENTORY.INVBAR
		+INVENTORY.ISHEALTH
	}
	
	//===========================================================================
	//
	// CreateCopy
	//
	//===========================================================================

	override Inventory CreateCopy (Actor other)
	{
		Inventory copy = Super.CreateCopy (other);
		copy.health = health;
		return copy;
	}

	//===========================================================================
	//
	// CreateTossable
	//
	//===========================================================================

	override Inventory CreateTossable (int amount)
	{
		Inventory copy = Super.CreateTossable (-1);
		if (copy != NULL)
		{
			copy.health = health;
		}
		return copy;
	}

	//===========================================================================
	//
	// HandlePickup
	//
	//===========================================================================

	override bool HandlePickup (Inventory item)
	{
		// HealthPickups that are the same type but have different health amounts
		// do not count as the same item.
		if (item.health == health)
		{
			return Super.HandlePickup (item);
		}
		return false;
	}

	//===========================================================================
	//
	// Use
	//
	//===========================================================================

	override bool Use (bool pickup)
	{
		return Owner.GiveBody (health, 0);
	}

	
}
