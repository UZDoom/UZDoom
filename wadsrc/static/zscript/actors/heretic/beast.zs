/*
** beast.zs
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

// Beast --------------------------------------------------------------------

class Beast : Actor
{
	Default
	{
		Health 220;
		Radius 32;
		Height 74;
		Mass 200;
		Speed 14;
		Painchance 100;
		Monster;
		+FLOORCLIP
		SeeSound "beast/sight";
		AttackSound "beast/attack";
		PainSound "beast/pain";
		DeathSound "beast/death";
		ActiveSound "beast/active";
		Obituary "$OB_BEAST";
		Tag "$FN_BEAST";
		DropItem "CrossbowAmmo", 84, 10;
	}
	States
	{
	Spawn:
		BEAS AB 10 A_Look;
		Loop;
	See:
		BEAS ABCDEF 3 A_Chase;
		Loop;
	Missile:
		BEAS H 10 A_FaceTarget;
		BEAS I 10 A_CustomComboAttack("BeastBall", 32, random[BeastAttack](1,8)*3, "beast/attack");
		Goto See;
	Pain:
		BEAS G 3;
		BEAS G 3 A_Pain;
		Goto See;
	Death:
		BEAS R 6;
		BEAS S 6 A_Scream;
		BEAS TUV 6;
		BEAS W 6 A_NoBlocking;
		BEAS XY 6;
		BEAS Z -1;
		Stop;
	XDeath:
		BEAS J 5;
		BEAS K 6 A_Scream;
		BEAS L 5;
		BEAS M 6;
		BEAS N 5;
		BEAS O 6 A_NoBlocking;
		BEAS P 5;
		BEAS Q -1;
		Stop;
	}
}			

// Beast ball ---------------------------------------------------------------

class BeastBall : Actor
{
	Default
	{
		Radius 9;
		Height 8;
		Speed 12;
		FastSpeed 20;
		Damage 4;
		Projectile;
		-ACTIVATEIMPACT
		-ACTIVATEPCROSS
		-NOBLOCKMAP
		+WINDTHRUST
		+SPAWNSOUNDSOURCE
		+ZDOOMTRANS
		RenderStyle "Add";
		SeeSound "beast/attack";
	}
	States
	{
	Spawn:
		FRB1 AABBCC 2 A_SpawnItemEx("Puffy", random2[BeastPuff]()*0.015625, random2[BeastPuff]()*0.015625, random2[BeastPuff]()*0.015625, 
									0,0,0,0,SXF_ABSOLUTEPOSITION, 64);
		Loop;
	Death:
		FRB1 DEFGH 4;
		Stop;
	}
}

// Puffy --------------------------------------------------------------------

class Puffy : Actor
{
	Default
	{
		Radius 6;
		Height 8;
		Speed 10;
		+NOBLOCKMAP
		+NOGRAVITY
		+MISSILE
		+NOTELEPORT
		+DONTSPLASH
		+ZDOOMTRANS
		RenderStyle "Add";
	}
	States
	{
	Spawn:
		FRB1 DEFGH 4;
		Stop;
	}
}



