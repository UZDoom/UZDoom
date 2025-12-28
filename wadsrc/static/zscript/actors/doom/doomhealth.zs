/*
** doomhealth.zs
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

// Health bonus -------------------------------------------------------------

class HealthBonus : Health
{
	Default
	{
		+COUNTITEM
		+INVENTORY.ALWAYSPICKUP
		Inventory.Amount 1;
		Inventory.MaxAmount 200;
		Inventory.PickupMessage "$GOTHTHBONUS";
		Tag "$TAG_HEALTHBONUS";
	}
	States
	{
	Spawn:
		BON1 ABCDCB 6;
		Loop;
	}

	//===========================================================================
	//
	// TryPickup
	//
	//===========================================================================

	override bool TryPickup (in out Actor other)
	{
		PrevHealth = other.player != NULL ? other.player.health : other.health;

		// Dehacked max health is compatibility dependent because Boom interpreted this value wrong.
		let maxamt = MaxAmount;
		if (maxamt < 0)
		{
			maxamt = deh.MaxHealth;
			if (!(Level.compatflags & COMPATF_DEHHEALTH)) maxamt *= 2;
		}

		if (other.GiveBody(Amount, maxamt))
		{
			GoAwayAndDie();
			return true;
		}
		return false;
	}

}
	
// Stimpack -----------------------------------------------------------------

class Stimpack : Health
{
	Default
	{
		Inventory.Amount 10;
		Inventory.PickupMessage "$GOTSTIM";
		Tag "$TAG_STIMPACK";
	}
	States
	{
	Spawn:
		STIM A -1;
		Stop;
	}
}

// Medikit -----------------------------------------------------------------

class Medikit : Health
{
	Default
	{
		Inventory.Amount 25;
		Inventory.PickupMessage "$GOTMEDIKIT";
		Health.LowMessage 25, "$GOTMEDINEED";
		Tag "$TAG_MEDIKIT";
	}
	States
	{
	Spawn:
		MEDI A -1;
		Stop;
	}
}
