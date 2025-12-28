/*
** strifeweapons.zs
**
**
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

class StrifeWeapon : Weapon
{
	Default
	{
		Weapon.Kickback 100;
	}
}

// Same as the bullet puff for Doom -----------------------------------------

class StrifePuff : Actor
{
	Default
	{
		+NOBLOCKMAP
		+NOGRAVITY
		+ALLOWPARTICLES
		RenderStyle "Translucent";
		Alpha 0.25;
	}

	States
	{
	Spawn:
		POW3 ABCDEFGH 3;
		Stop;
	Crash:
		PUFY A 4 Bright;
		PUFY BCD 4;
		Stop;
	}
}
	

// A spark when you hit something that doesn't bleed ------------------------
// Only used by the dagger.

class StrifeSpark : StrifePuff
{
	Default
	{
		+ZDOOMTRANS
		RenderStyle "Add";
	}
	States
	{
	Crash:
		POW2 ABCD 4;
		Stop;
	}
}

