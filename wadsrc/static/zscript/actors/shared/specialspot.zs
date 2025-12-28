/*
** specialspot.zs
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

class SpotState : Object native play
{
	deprecated ("3.8") static SpotState GetSpotState(bool create = true)
	{
		return level.GetSpotState(create);
	}
	native SpecialSpot GetNextInList(class<Actor> type, int skipcounter);
	native SpecialSpot GetSpotWithMinMaxDistance(Class<Actor> type, double x, double y, double mindist, double maxdist);
	native SpecialSpot GetRandomSpot(class<Actor> type, bool onlyonce);
	native void AddSpot(SpecialSpot spot);
	native void RemoveSpot(SpecialSpot spot);
	
}

class SpecialSpot : Actor
{
	override void BeginPlay()
	{
		let sstate = Level.GetSpotState();
		if (sstate != NULL) sstate.AddSpot(self);
		Super.BeginPlay();
	}

	//----------------------------------------------------------------------------
	//
	//
	//
	//----------------------------------------------------------------------------

	override void OnDestroy()
	{
		let sstate = Level.GetSpotState(false);
		if (sstate != NULL) sstate.RemoveSpot(self);
		Super.OnDestroy();
	}
	
	// Mace spawn spot ----------------------------------------------------------

	// Every mace spawn spot will execute this action. The first one
	// will build a list of all mace spots in the level and spawn a
	// mace. The rest of the spots will do nothing.

	void A_SpawnSingleItem(class<Actor> cls, int fail_sp = 0, int fail_co = 0, int fail_dm = 0)
	{
		Actor spot = NULL;
		let state = Level.GetSpotState();

		if (state != NULL) spot = state.GetRandomSpot(GetClass(), true);
		if (spot == NULL) return;

		if (!multiplayer && random[SpawnMace]() < fail_sp)
		{ // Sometimes doesn't show up if not in deathmatch
			return;
		}

		if (multiplayer && !deathmatch && random[SpawnMace]() < fail_co)
		{
			return;
		}

		if (deathmatch && random[SpawnMace]() < fail_dm)
		{
			return;
		}

		if (cls == NULL)
		{
			return;
		}

		let spawned = Spawn(cls, self.Pos, ALLOW_REPLACE);

		if (spawned)
		{
			// Setting z in two steps is necessary to proper initialize floorz before using it.
			spawned.SetOrigin (spot.Pos, false);
			spawned.SetZ(spawned.floorz);
			// We want this to respawn.
			if (!bDropped)
			{
				spawned.bDropped = false;
			}
			let inv = Inventory(spawned);
			if (inv)
			{
				inv.SpawnPointClass = GetClass();
			}
		}
	}


}
