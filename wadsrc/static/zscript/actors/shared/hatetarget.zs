/*
** hatetarget.zs
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

// Hate Target --------------------------------------------------------------

class HateTarget : Actor
{
	default
	{
		Radius 20;
		Height 56;
		+SHOOTABLE
		+NOGRAVITY
		+NOBLOOD
		+DONTSPLASH
		Mass 0x7fffffff;
	}
	States
	{
	Spawn:
		TNT1 A -1;
	}
	
	override void BeginPlay()
	{
		Super.BeginPlay();
		if (SpawnAngle != 0)
		{	// Each degree translates into 10 units of health
			health = SpawnAngle * 10;
		}
		else
		{
			special2 = 1;
			health = 1000001;
		}
	}

	override int TakeSpecialDamage(Actor inflictor, Actor source, int damage, Name damagetype, int flags, double angle)
	{
		if (special2 != 0)
		{
			return 0;
		}
		else
		{
			return damage;
		}
	}

	
}
