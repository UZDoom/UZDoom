/*
** mapmarker.zs
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

// Map Marker --------------------------------------------------------------
//
// This class uses the following argument:
//   args[0] == 0, shows the sprite at this actor
//           != 0, shows the sprite for all actors whose TIDs match instead
//
//   args[1] == 0, show the sprite always
//           == 1, show the sprite only after its sector has been drawn
//
//   args[2] == 0, show the sprite with a constant scale
//           == 1, show the sprite with a scale relative to automap zoom
//
// To enable display of the sprite, activate it. To turn off the sprite,
// deactivate it.
//
// All the code to display it is in am_map.cpp.
//
//--------------------------------------------------------------------------

class MapMarker : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
		+INVISIBLE
		Scale 0.5;
	}
	States
	{
	Spawn:
		AMRK A -1;
		Stop;
	}
	
	override void BeginPlay ()
	{
		ChangeStatNum (STAT_MAPMARKER);
	}

	override void Activate (Actor activator)
	{
		bDormant = true;
	}

	override void Deactivate (Actor activator)
	{
		bDormant = false;
	}
	
}
