/*
** scriptutil.zs
**
** Container for utility functions used by ACS and FraggleScript.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
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

class ScriptUtil play
{

	//============================================================================
	//
	// GiveInventory
	//
	// Gives an item to one or more actors.
	//
	//============================================================================

	static void GiveInventory (Actor activator, Name type, int amount)
	{
		if (amount <= 0 || type == 'none')
		{
			return;
		}
		if (type == 'Armor')
		{
			type = "BasicArmorPickup";
		}
		Class<Actor> info = type;
		if (info == NULL)
		{
			Console.Printf ("GiveInventory: Unknown item type %s.\n", type);
		}
		else if (!(info is 'Inventory'))
		{
			Console.Printf ("GiveInventory: %s is not an inventory item.\n", type);
		}
		else if (activator == NULL)
		{
			for (int i = 0; i < MAXPLAYERS; ++i)
			{
				if (playeringame[i])
					players[i].mo.GiveInventory((class<Inventory>)(info), amount);
			}
		}
		else
		{
			activator.GiveInventory((class<Inventory>)(info), amount);
		}
	}

	//============================================================================
	//
	// TakeInventory
	//
	// Takes an item from one or more actors.
	//
	//============================================================================

	static void TakeInventory (Actor activator, Name type, int amount)
	{
		if (type == 'none')
		{
			return;
		}
		if (type == 'Armor')
		{
			type = Actor.GetBasicArmorClass().GetClassName();
		}
		if (amount <= 0)
		{
			return;
		}
		Class<Inventory> info = type;
		if (info == NULL)
		{
			return;
		}
		if (activator == NULL)
		{
			for (int i = 0; i < MAXPLAYERS; ++i)
			{
				if (playeringame[i])
					players[i].mo.TakeInventory(info, amount);
			}
		}
		else
		{
			activator.TakeInventory(info, amount);
		}
	}

	//============================================================================
	//
	// ClearInventory
	//
	// Clears the inventory for one or more actors.
	//
	//============================================================================

	static void ClearInventory (Actor activator)
	{
		if (activator == NULL)
		{
			for (int i = 0; i < MAXPLAYERS; ++i)
			{
				if (playeringame[i])
					players[i].mo.ClearInventory();
			}
		}
		else
		{
			activator.ClearInventory();
		}
	}
	
	//==========================================================================
	//
	//
	//
	//==========================================================================

	static int SetWeapon(Actor activator, class<Inventory> cls)
	{
		if(activator != NULL && activator.player != NULL && cls != null)
		{
			let item = Weapon(activator.FindInventory(cls));

			if(item != NULL)
			{
				if(activator.player.ReadyWeapon == item)
				{
					// The weapon is already selected, so setweapon succeeds by default,
					// but make sure the player isn't switching away from it.
					activator.player.PendingWeapon = WP_NOCHANGE;
					return 1;
				}
				else
				{
					if(item.CheckAmmo(Weapon.EitherFire, false))
					{
						// There's enough ammo, so switch to it.
						activator.player.PendingWeapon = item;
						return 1;
					}
				}
			}
		}
		return 0;
	}

	//==========================================================================
	//
	//
	//
	//==========================================================================

	static void SetMarineWeapon(LevelLocals Level, Actor activator, int tid, int marineweapontype)
	{
		if (tid != 0)
		{
			let it = Level.CreateActorIterator(tid, 'ScriptedMarine');
			ScriptedMarine marine;

			while ((marine = ScriptedMarine(it.Next())) != NULL)
			{
				marine.SetWeapon(marineweapontype);
			}
		}
		else
		{
			let marine = ScriptedMarine(activator);
			if (marine != null)
			{
				marine.SetWeapon(marineweapontype);
			}
		}
	}

	//==========================================================================
	//
	//
	//
	//==========================================================================

	static void SetMarineSprite(LevelLocals Level, Actor activator, int tid, class<Actor> type)
	{
		if (type != NULL)
		{
			if (tid != 0)
			{
				let it = Level.CreateActorIterator(tid, 'ScriptedMarine');
				ScriptedMarine marine;

				while ((marine = ScriptedMarine(it.Next())) != NULL)
				{
					marine.SetSprite(type);
				}
			}
			else
			{
				let marine = ScriptedMarine(activator);
				if (marine != null)
				{
					marine.SetSprite(type);
				}
			}
		}
		else
		{
			Console.Printf ("Unknown actor type: %s\n", type.GetClassName());
		}
	}
	

	//==========================================================================
	//
	//
	//
	//==========================================================================

	static int PlayerMaxAmmo(Actor activator, class<Actor> type, int newmaxamount = int.min, int newbpmaxamount = int.min)
	{
		if (activator == null) return 0;
		let ammotype = (class<Ammo>)(type);
		if (ammotype == null) return 0;
		
		if (newmaxamount != int.min)
		{
			let iammo = Ammo(activator.FindInventory(ammotype));
			if(newmaxamount < 0) newmaxamount = 0;
			if (!iammo) 
			{
				activator.GiveAmmo(ammotype, 1);
				iammo = Ammo(activator.FindInventory(ammotype));
				if (iammo)
					iammo.Amount = 0;
			}

			for (Inventory item = activator.Inv; item != NULL; item = item.Inv)
			{
				if (item is 'BackpackItem')
				{
					if (newbpmaxamount == int.min) newbpmaxamount = newmaxamount * 2;
					break;
				}
			}
			if (iammo)
			{
				iammo.MaxAmount = newmaxamount;
				iammo.BackpackMaxAmount = newbpmaxamount;
			}
		}

		let rammo = activator.FindInventory(ammotype);
		if (rammo) return rammo.maxamount;
		else return GetDefaultByType(ammotype).MaxAmount;
	}

	//==========================================================================
	//
	//
	//
	//==========================================================================

	static int PlayerAmmo(Actor activator, class<Inventory> type, int newamount = int.min)
	{
		if (activator == null) return 0;
		let ammotype = (class<Ammo>)(type);
		if (ammotype == null) return 0;
		
		if (newamount != int.min)
		{
			let iammo = activator.FindInventory(ammotype);
			newamount = max(newamount, 0);
			if (iammo) iammo.Amount = newamount;
			else activator.GiveAmmo(ammotype, newamount);
		}
		let iammo = activator.FindInventory(ammotype);
		if (iammo) return iammo.Amount;
		else return 0;
	}	
	
}
