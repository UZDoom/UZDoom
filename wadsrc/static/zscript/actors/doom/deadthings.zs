/*
** deadthings.zs
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

// Gibbed marine -----------------------------------------------------------

class GibbedMarine : Actor
{
	States
	{
	Spawn:
		PLAY W -1;
		Stop;
	}
}

// Gibbed marine (extra copy) ----------------------------------------------

class GibbedMarineExtra : GibbedMarine
{
}

// Dead marine -------------------------------------------------------------

class DeadMarine : Actor
{
	States
	{
	Spawn:
		PLAY N -1;
		Stop;
	}
}

/* If it wasn't for Dehacked compatibility, the rest of these would be
 * better defined as single frame states. But since Doom reused the
 * dead state from the original monsters, we need to do the same.
 */

// Dead zombie man ---------------------------------------------------------

class DeadZombieMan : ZombieMan
{
	Default
	{
		Skip_Super;
		DropItem "None";
	}
	States
	{
	Spawn:
		Goto Super::Death+4;
	}
}

// Dead shotgun guy --------------------------------------------------------

class DeadShotgunGuy : ShotgunGuy
{
	Default
	{
		Skip_Super;
		DropItem "None";
	}
	States
	{
	Spawn:
		Goto Super::Death+4;
	}
}

// Dead imp ----------------------------------------------------------------

class DeadDoomImp : DoomImp
{
	Default
	{
		Skip_Super;
	}
	States
	{
	Spawn:
		Goto Super::Death+4;
	}
}

// Dead demon --------------------------------------------------------------

class DeadDemon : Demon
{
	Default
	{
		Skip_Super;
	}
	States
	{
	Spawn:
		Goto Super::Death+5;
	}
}

// Dead cacodemon ----------------------------------------------------------

class DeadCacodemon : Cacodemon
{
	Default
	{
		Skip_Super;
	}
	States
	{
	Spawn:
		Goto Super::Death+5;
	}
}

// Dead lost soul ----------------------------------------------------------

/* [RH] Considering that the lost soul removes itself when it dies, there
 * really wasn't much point in id including this thing, but they did anyway.
 * (There was probably a time when it stayed around after death, and this is
 * a holdover from that.)
 */

class DeadLostSoul : LostSoul
{
	Default
	{
		Skip_Super;
	}
	States
	{
	Spawn:
		Goto Super::Death+5;
	}
}
