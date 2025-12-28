/*
** dog.zs
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

class MBFHelperDog : Actor
{
	default
	{
		Health 500;
		Speed 10;
		PainChance 180;
		Radius 12;
		Height 28;
		Mass 100;
		Monster;
		+JUMPDOWN
		ActiveSound "dog/active";
		AttackSound "dog/attack";
		DeathSound "dog/death";
		PainSound "dog/pain";
		SeeSound "dog/sight";
		Obituary "$OB_DOG";
		Tag "$FN_DOG";
	}
	States
	{
	Spawn:
		DOGS AB 10 A_Look;
		Loop;
	See:
		DOGS AABBCCDD 2 A_Chase;
		Loop;
	Melee:
		DOGS EF 8 A_FaceTarget;
		DOGS G 8 A_SargAttack;
		Goto See;
	Pain:
		DOGS H 2;
		DOGS H 2 A_Pain;
		Goto See;
	Death:
		DOGS I 8;
		DOGS J 8 A_Scream;
		DOGS K 4;
		DOGS L 4 A_Fall;
		DOGS M 4;
		DOGS N -1;
	Raise:
		DOGS NMLKJI 5;
		Goto See;
	}
}
