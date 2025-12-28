/*
** svestuff.zs
**
** CTC flag spots. They are not functional and only here so that
** something gets spawned for them.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Rogue Entertainment
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

class SVEFlagSpotRed : Inventory
{
	States
	{
	Spawn:
		FLGR A 1 BRIGHT;
		FLGR A 0 BRIGHT; // A_FlagStandSpawn
		FLGR A 1 BRIGHT; // A_FlagStandCheck
		Wait;
	}
}

class SVEFlagSpotBlue : Inventory
{
	States
	{
	Spawn:
		FLGB A 1 BRIGHT;
		FLGB A 0 BRIGHT; // A_FlagStandSpawn
		FLGB A 1 BRIGHT; // A_FlagStandCheck
		Wait;
	}
}

class SVEBlueChalice : Inventory
{
	Default
	{
		+DROPPED
		+FLOORCLIP
		+INVENTORY.INVBAR
		Radius 10;
		Height 16;
		Tag "$TAG_OFFERINGCHALICE";
		Inventory.Icon "I_RELB";
		Inventory.PickupMessage "$TXT_OFFERINGCHALICE";
	}
	States
	{
	Spawn:
		RELB A -1 BRIGHT;
		Stop;
	}
}

class SVETalismanPowerup : Inventory
{
}

class SVETalismanRed : Inventory
{
	Default
	{
		+DROPPED
		+FLOORCLIP
		+INVENTORY.INVBAR
		Radius 10;
		Height 16;
		Inventory.MaxAmount 1;
		Inventory.Icon "I_FLGR";
		Tag "$TAG_TALISMANRED";
		Inventory.PickupMessage "$MSG_TALISMANRED";
	}
	States
	{
	Spawn:
		FLGR A -1 BRIGHT;
		Stop;
	}
	
	override bool TryPickup (in out Actor toucher)
	{
		let useok = Super.TryPickup (toucher);
		if (useok)
		{
			if (toucher.FindInventory("SVETalismanRed") && 
				toucher.FindInventory("SVETalismanGreen") && 
				toucher.FindInventory("SVETalismanBlue"))
			{
				toucher.A_Print("$MSG_TALISMANPOWER");
				toucher.GiveInventoryType("SVETalismanPowerup");
			}
		}
		return useok;
	}
	
}

class SVETalismanBlue : SVETalismanRed
{
	Default
	{
		+INVENTORY.INVBAR
		Inventory.Icon "I_FLGB";
		Tag "$TAG_TALISMANBLUE";
		Inventory.PickupMessage "$MSG_TALISMANBLUE";
	}
	States
	{
	Spawn:
		FLGB A -1 BRIGHT;
		Stop;
	}
}

class SVETalismanGreen : SVETalismanRed
{
	Default
	{
		+INVENTORY.INVBAR
		Inventory.Icon "I_FLGG";
		Tag "$TAG_TALISMANGREEN";
		Inventory.PickupMessage "$MSG_TALISMANGREEN";
	}
	States
	{
	Spawn:
		FLGG A -1 BRIGHT;
		Stop;
	}
}

class SVEOreSpawner : Actor
{
	Default
	{
		+NOSECTOR
	}
	States
	{
	Spawn:
		TNT1 A 175 A_OreSpawner;
		loop;
	}
	
	//
	// A_OreSpawner
	//
	// [SVE] svillarreal
	//

	void A_OreSpawner()
	{
		if(deathmatch) return;

		bool inrange = false;
		for(int i = 0; i < MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;

			if(Distance2D(players[i].mo) < 2048)
			{
				inrange = true;
				break;
			}
		}
		if(!inrange) return;

		let it = ThinkerIterator.Create("DegninOre");
		Thinker ac;
		
		int numores = 0;
		while (ac = it.Next())
		{
			if (++numores == 3) return;
		}
		Spawn("DegninOre", Pos);
	}
}

class SVEOpenDoor225 : DummyStrifeItem 
{
	override bool TryPickup (in out Actor toucher)
	{
		Door_Open(225, 16);
		GoAwayAndDie ();
		return true;
	}
	
	override bool SpecialDropAction (Actor dropper)
	{
		Door_Open(225, 16);
		Destroy ();
		return true;
	}
	
}
