/*
** blood.zs
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

// Blood sprite ------------------------------------------------------------

class Blood : Actor
{
	Default
	{
		Mass 5;
		+NOBLOCKMAP
		+NOTELEPORT
		+ALLOWPARTICLES
	}
	States
	{
	Spawn:
		BLUD CBA 8;
		Stop;
	Spray:
		SPRY ABCDEF 3;
		SPRY G 2;
		Stop;
	}
}

// Blood splatter -----------------------------------------------------------

class BloodSplatter : Actor
{
	Default
	{
		Radius 2;
		Height 4;
		+NOBLOCKMAP
		+MISSILE
		+DROPOFF
		+NOTELEPORT
		+CANNOTPUSH
		+ALLOWPARTICLES
		Mass 5;
	}
	States
	{
	Spawn:
		BLUD CBA 8;
		Stop;
	Death:
		BLUD A 6;
		Stop;
	}
}
	
// Axe Blood ----------------------------------------------------------------

class AxeBlood : Actor
{
	Default
	{
		Radius 2;
		Height 4;
		+NOBLOCKMAP
		+NOGRAVITY
		+DROPOFF
		+NOTELEPORT
		+CANNOTPUSH
		+ALLOWPARTICLES
		Mass 5;
	}
	States
	{
	Spawn:
		FAXE FGHIJ 3;
	Death:
		FAXE K 3;
		Stop;
	}
}

	
