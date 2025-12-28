/*
** id24mindweaver.zs
**
** id1 - mindweaver
**
**---------------------------------------------------------------------------
**
** Copyright 1993-2024 id Software LLC, a ZeniMax Media company.
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
** converted from DECOHACK
*/

class ID24Mindweaver : Actor
{
	Default
	{
		Health 500;
		Speed 12;
		Radius 64;
		Height 64;
		ReactionTime 8;
		PainChance 40;
		Mass 600;
		Monster;
		SeeSound "monsters/mindweaver/sight";
		PainSound "monsters/mindweaver/pain";
		DeathSound "monsters/mindweaver/death";
		ActiveSound "monsters/mindweaver/active";
		Obituary "$ID24_OB_MINDWEAVER";
		Tag "$ID24_CC_MINDWEAVER";
	}
	States
	{
	Spawn:
		CSPI AB 10 A_Look;
		Loop;
	See:
		CSPI A 20;
	Walk:
		CSPI A 0 A_StartSound("monsters/mindweaver/walk");
		CSPI AABBCC 3 A_Chase;
		CSPI D 0 A_StartSound("monsters/mindweaver/walk");
		CSPI DDEEFF 3 A_Chase;
		Loop;
	Missile:
		CSPI A 20 BRIGHT A_FaceTarget;
	Fighting:
		CSPI G 4 BRIGHT A_SPosAttack;
		CSPI H 4 BRIGHT A_SPosAttack;
		CSPI H 1 BRIGHT A_SpidRefire;
		Goto Fighting;
	Pain:
		CSPI I 3;
		CSPI I 3 A_Pain;
		Goto Walk;
	Death:
		CSPI J 20 A_Scream;
		CSPI K 7 A_Fall;
		CSPI LMNO 7;
		CSPI P -1 A_BossDeath;
		Stop;
	Raise:
		CSPI P 5;
		CSPI ONMLKJ 5;
		Goto Walk;
	}
}
