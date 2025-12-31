/*
** snake.zs
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

class Snake : Actor
{
	Default
	{
		Health 280;
		Radius 22;
		Height 70;
		Speed 10;
		Painchance 48;
		Monster;
		+FLOORCLIP
		AttackSound "snake/attack";
		SeeSound "snake/sight";
		PainSound "snake/pain";
		DeathSound "snake/death";
		ActiveSound "snake/active";
		Obituary "$OB_SNAKE";
		Tag "$FN_SNAKE";
		DropItem "PhoenixRodAmmo", 84, 5;
	}
	States
	{
	Spawn:
		SNKE AB 10 A_Look;
		Loop;
	See:
		SNKE ABCD 4 A_Chase;
		Loop;
	Missile:
		SNKE FF 5 A_FaceTarget;
		SNKE FFF 4 A_SpawnProjectile("SnakeProjA", 32, 0, 0, CMF_CHECKTARGETDEAD);
		SNKE FFF 5 A_FaceTarget;
		SNKE F 4 A_SpawnProjectile("SnakeProjB", 32, 0, 0, CMF_CHECKTARGETDEAD);
		Goto See;
	Pain:
		SNKE E 3;
		SNKE E 3 A_Pain;
		Goto See;
	Death:
		SNKE G 5;
		SNKE H 5 A_Scream;
		SNKE IJKL 5;
		SNKE M 5 A_NoBlocking;
		SNKE NO 5;
		SNKE P -1;
		Stop;
	}
}

// Snake projectile A -------------------------------------------------------

class SnakeProjA : Actor
{
	Default
	{
		Radius 12;
		Height 8;
		Speed 14;
		FastSpeed 20;
		Damage 1;
		Projectile;
		-NOBLOCKMAP
		-ACTIVATEIMPACT
		-ACTIVATEPCROSS
		+WINDTHRUST
		+SPAWNSOUNDSOURCE
		+ZDOOMTRANS
		RenderStyle "Add";
		SeeSound "snake/attack";
	}
	States
	{
	Spawn:
		SNFX ABCD 5 Bright;
		Loop;
	Death:
		SNFX EF 5 Bright;
		SNFX G 4 Bright;
		SNFX HI 3 Bright;
		Stop;
	}
}	

// Snake projectile B -------------------------------------------------------

class SnakeProjB : SnakeProjA
{
	Default
	{
		Damage 3;
		+NOBLOCKMAP
		-WINDTHRUST
	}
	States
	{
	Spawn:
		SNFX JK 6 Bright;
		Loop;
	Death:
		SNFX LM 5 Bright;
		SNFX N 4 Bright;
		SNFX O 3 Bright;
		Stop;
	}
}	
