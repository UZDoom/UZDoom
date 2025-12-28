/*
** inv_misc.zs
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

//===========================================================================
//
//
//
//===========================================================================

class ScoreItem : Inventory
{
	Default
	{
		Height 10;
		+COUNTITEM
		Inventory.Amount 1;
		+Inventory.ALWAYSPICKUP
	}
	
	override bool TryPickup (in out Actor toucher)
	{
		toucher.Score += Amount;
		GoAwayAndDie();
		return true;
	}
}

//===========================================================================
//
//
//
//===========================================================================

class Key : Inventory
{
	Default
	{
		+DONTGIB;		// Don't disappear due to a crusher
		+INVENTORY.ISKEYITEM;
		Inventory.InterHubAmount 0;
		Inventory.PickupSound "misc/k_pkup";
	}

	static native clearscope bool IsLockDefined(int locknum);
	static native clearscope Color GetMapColorForLock(int locknum);
	static native clearscope Color GetMapColorForKey(Key key);
	static native clearscope int GetKeyTypeCount();
	static native clearscope class<Key> GetKeyType(int index);

	override bool ShouldShareItem(Actor giver)
	{
		return sv_coopsharekeys;
	}
	
	override bool HandlePickup (Inventory item)
	{
		// In single player, you can pick up an infinite number of keys
		// even though you can only hold one of each.
		if (multiplayer)
		{
			return Super.HandlePickup (item);
		}
		if (GetClass() == item.GetClass())
		{
			item.bPickupGood = true;
			return true;
		}
		return false;
	}

	override bool ShouldStay ()
	{
		return !!multiplayer;
	}
}

//===========================================================================
//
// AMapRevealer
//
// A MapRevealer reveals the whole map for the player who picks it up.
// The MapRevealer doesn't actually go in your inventory. Instead, it sets
// a flag on the level.
//
//===========================================================================

class MapRevealer : Inventory
{
	override bool TryPickup (in out Actor toucher)
	{
		level.allmap = true;
		GoAwayAndDie ();
		return true;
	}
}

//===========================================================================
//
//
//
//===========================================================================

class PuzzleItem : Inventory
{
	meta int PuzzleItemNumber;
	meta String PuzzFailMessage;
	meta Sound PuzzFailSound;
	
	property Number: PuzzleItemNumber;
	property FailMessage: PuzzFailMessage;
	property FailSound: PuzzFailSound;

	Default
	{
		+NOGRAVITY
		+INVENTORY.INVBAR
		+INVENTORY.ISKEYITEM
		Inventory.DefMaxAmount;
		Inventory.UseSound "PuzzleSuccess";
		Inventory.PickupSound "misc/i_pkup";
		PuzzleItem.FailMessage("$TXT_USEPUZZLEFAILED");
		PuzzleItem.FailSound "*puzzfail";
	}

	override bool ShouldShareItem(Actor giver)
	{
		return sv_coopsharekeys;
	}
	
	override bool HandlePickup (Inventory item)
	{
		// Can't carry more than 1 of each puzzle item in coop netplay
		if (multiplayer && !deathmatch && item.GetClass() == GetClass())
		{
			return true;
		}
		return Super.HandlePickup (item);
	}

	override bool Use (bool pickup)
	{
		if (Owner == NULL) return false;
		if (Owner.UsePuzzleItem (PuzzleItemNumber))
		{
			return true;
		}
		// [RH] Always play the sound if the use fails.
		Owner.A_StartSound (PuzzFailSound, CHAN_VOICE);
		if (Owner.CheckLocalView())
		{
			Console.MidPrint (null, PuzzFailMessage, true);
		}
		return false;
	}

	override void UseAll(Actor user)
	{
	}
	
	override bool ShouldStay ()
	{
		return !!multiplayer;
	}

}

