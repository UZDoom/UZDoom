/*
** id24banshee.zs
**
** id1 - banshee
**
**---------------------------------------------------------------------------
**
** Copyright 1993-2024 id Software LLC, a ZeniMax Media company.
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
** converted from DECOHACK
*/

class ID24Banshee : Actor
{
	Default
	{
		Health 100;
		Speed 8;
		Radius 20;
		Height 56;
		ReactionTime 8;
		PainChance 64;
		Mass 500;
		Monster;
		+FLOAT
		+NOGRAVITY
		SeeSound "monsters/banshee/sight";
		PainSound "monsters/banshee/pain";
		DeathSound "monsters/banshee/death";
	//	SelfObituary "$ID24_OB_BANSHEE";
		Tag "$ID24_CC_BANSHEE";
	}
	States
	{
	Spawn:
		BSHE AB 10 BRIGHT A_Look;
		Loop;
	See:
		BSHE AAABBBCCCAAABBBCCC 2 BRIGHT A_Chase;
		BSHE A 0 BRIGHT A_StartSound("monsters/banshee/active");
		Loop;
	Melee:
		BSHE D 1 BRIGHT A_RadiusDamage(100, 8);
		Wait;
	Pain:
		BSHE D 3 BRIGHT;
		BSHE D 3 BRIGHT A_Pain;
		Goto See;
	Death:
		BSHE D 4 BRIGHT A_Scream;
		BSHE E 6 BRIGHT A_RadiusDamage(128, 128);
		BSHE F 8 BRIGHT A_Fall;
		BSHE G 6 BRIGHT;
		BSHE H 4 BRIGHT;
		TNT1 A 20;
		Stop;
	}
}
