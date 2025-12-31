/*
** mummy.zs
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

// Mummy --------------------------------------------------------------------

class Mummy : Actor
{
	Default
	{
		Health 80;
		Radius 22;
		Height 62;
		Mass 75;
		Speed 12;
		Painchance 128;
		Monster;
		+FLOORCLIP
		SeeSound "mummy/sight";
		AttackSound "mummy/attack1";
		PainSound "mummy/pain";
		DeathSound "mummy/death";
		ActiveSound "mummy/active";
		HitObituary "$OB_MUMMY";
		Tag "$FN_MUMMY";
		DropItem "GoldWandAmmo", 84, 3;
	}
	States
	{
	Spawn:
		MUMM AB 10 A_Look;
		Loop;
	See:
		MUMM ABCD 4 A_Chase;
		Loop;
	Melee:
		MUMM E 6 A_FaceTarget;
		MUMM F 6 A_CustomMeleeAttack(random[MummyAttack](1,8)*2, "mummy/attack2", "mummy/attack");
		MUMM G 6;
		Goto See;
	Pain:
		MUMM H 4;
		MUMM H 4 A_Pain;
		Goto See;
	Death:
		MUMM I 5;
		MUMM J 5 A_Scream;
		MUMM K 5 A_SpawnItemEx("MummySoul", 0,0,10, 0,0,1);
		MUMM L 5;
		MUMM M 5 A_NoBlocking;
		MUMM NO 5;
		MUMM P -1;
		Stop;
	}
}

// Mummy leader -------------------------------------------------------------

class MummyLeader : Mummy
{
	Default
	{
		Species "MummyLeader";
		Health 100;
		Painchance 64;
		Obituary "$OB_MUMMYLEADER";
		Tag "$FN_MUMMYLEADER";
	}
	States
	{
	Missile:
		MUMM X 5 A_FaceTarget;
		MUMM Y 5 Bright A_FaceTarget;
		MUMM X 5 A_FaceTarget;
		MUMM Y 5 Bright A_FaceTarget;
		MUMM X 5 A_FaceTarget;
		MUMM Y 5 Bright A_CustomComboAttack("MummyFX1", 32, random[MummyAttack2](1,8)*2, "mummy/attack2");
		Goto See;
	}
}

// Mummy ghost --------------------------------------------------------------

class MummyGhost : Mummy
{
	Default
	{
		+SHADOW
		+GHOST
		RenderStyle "Translucent";
		Alpha 0.4;
	}
}

// Mummy leader ghost -------------------------------------------------------

class MummyLeaderGhost : MummyLeader
{
	Default
	{
		Species "MummyLeaderGhost";
		+SHADOW
		+GHOST
		RenderStyle "Translucent";
		Alpha 0.4;
	}
}

// Mummy soul ---------------------------------------------------------------

class MummySoul : Actor
{
	Default
	{
		+NOBLOCKMAP
		+NOGRAVITY
	}
	States
	{
	Spawn:
		MUMM QRS 5;
		MUMM TUVW 9;
		Stop;
	}
}

// Mummy FX 1 (flying head) -------------------------------------------------

class MummyFX1 : Actor
{
	Default
	{
		Radius 8;
		Height 14;
		Speed 9;
		FastSpeed 18;
		Damage 4;
		RenderStyle "Add";
		Projectile;
		-ACTIVATEPCROSS
		-ACTIVATEIMPACT
		+SEEKERMISSILE
		+ZDOOMTRANS
	}
	States
	{
	Spawn:
		FX15 A 5 Bright A_StartSound("mummy/head");
		FX15 B 5 Bright A_SeekerMissile(10,20);
		FX15 C 5 Bright;
		FX15 B 5 Bright A_SeekerMissile(10,20);
		Loop;
	Death:
		FX15 DEFG 5 Bright;
		Stop;
	}
}
