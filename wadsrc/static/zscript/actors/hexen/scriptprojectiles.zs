/*
** scriptprojectiles.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
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

// Fire Ball ----------------------------------------------------------------

class FireBall : Actor
{
	Default
	{
		Speed 2;
		Radius 8;
		Height 8;
		Damage 4;
		DamageType "Fire";
		+NOBLOCKMAP +NOGRAVITY +DROPOFF +MISSILE
		+NOTELEPORT
		+ZDOOMTRANS
		RenderStyle "Add";
		DeathSound "Fireball";
	}
	States
	{
	Spawn:
		FBL1 AB 4 Bright;
		Loop;
	Death:
		XPL1 ABCDEF 4 Bright;
		Stop;
	}
}

// Arrow --------------------------------------------------------------------

class Arrow : Actor
{
	Default
	{
		Speed 6;
		Radius 8;
		Height 4;
		Damage 4;
		+NOBLOCKMAP +NOGRAVITY +DROPOFF +MISSILE
		+NOTELEPORT
	}
	States
	{
	Spawn:
		ARRW A -1;
		Stop;
	Death:
		ARRW A 1;
		Stop;
	}
}

// Dart ---------------------------------------------------------------------

class Dart : Actor
{
	Default
	{
		Speed 6;
		Radius 8;
		Height 4;
		Damage 2;
		+NOBLOCKMAP +NOGRAVITY +DROPOFF +MISSILE
		+NOTELEPORT
	}
	States
	{
	Spawn:
		DART A -1;
		Stop;
	Death:
		DART A 1;
		Stop;
	}
}

// Poison Dart --------------------------------------------------------------

class PoisonDart : Dart
{
	Default
	{
		PoisonDamage 20;
	}
}

// Ripper Ball --------------------------------------------------------------

class RipperBall : Actor
{
	Default
	{
		Speed 6;
		Radius 8;
		Damage 2;
		+NOBLOCKMAP +NOGRAVITY +DROPOFF +MISSILE
		+NOTELEPORT +RIPPER
	}
	States
	{
	Spawn:
		RIPP ABC 3;
		Loop;
	Death:
		CFCF Q 4 Bright;
		CFCF R 3 Bright;
		CFCF S 4 Bright;
		CFCF T 3 Bright;
		CFCF U 4 Bright;
		CFCF V 3 Bright;
		CFCF W 4 Bright;
		CFCF X 3 Bright;
		CFCF Y 4 Bright;
		CFCF Z 3 Bright;
		Stop;
	}
}

// Projectile Blade ---------------------------------------------------------

class ProjectileBlade : Actor
{
	Default
	{
		Speed 6;
		Radius 6;
		Height 6;
		Damage 3;
		+NOBLOCKMAP +NOGRAVITY +DROPOFF +MISSILE
		+NOTELEPORT
	}
	States
	{
	Spawn:
		BLAD A -1;
		Stop;
	Death:
		BLAD A 1;
		Stop;
	}
}

