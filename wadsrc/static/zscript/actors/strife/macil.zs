/*
** macil.zs
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

// Macil (version 1) ---------------------------------------------------------

class Macil1 : Actor
{
	Default
	{
		Health 95;
		Radius 20;
		Height 56;
		Speed 8;
		Painchance 250;
		Monster;
		-COUNTKILL
		+NOTDMATCH
		+NOICEDEATH
		+NOSPLASHALERT
		+NODAMAGE
		+NEVERRESPAWN
		DamageFactor "Fire", 0.5;
		MinMissileChance 150;
		SeeSound "macil/sight";
		PainSound "macil/pain";
		ActiveSound "macil/active";
		CrushPainSound "misc/pcrush";
		Tag "$TAG_MACIL1";
		Obituary "$OB_MACIL";
		DropItem "BoxOfBullets";
		MaxStepHeight 16;
		MaxDropoffHeight 32;
	}
	States
	{
	Spawn:
		LEDR C 5 A_Look2;
		Loop;
		LEDR A 8;
		Loop;
		LEDR B 8;
		Loop;
		LEAD ABCD 6 A_Wander;
		Loop;
	See:
		LEAD AABBCCDD 3 A_Chase;
		Loop;
	Missile:
	Death:
		LEAD E 2 A_FaceTarget;
		LEAD F 2 BRIGHT A_ShootGun;
		LEAD E 1 A_SentinelRefire;
		Loop;
	Pain:
		LEAD Y 3;
		LEAD Y 3 A_Pain;
		Goto See;
	}
}
		

// Macil (version 2) ---------------------------------------------------------

class Macil2 : Macil1
{
	Default
	{
		Painchance 200;
		+COUNTKILL
		+SPECTRAL
		-NODAMAGE
		Tag "$TAG_MACIL2";
		DeathSound "macil/slop";
		DropItem "None";
		DamageFactor "SpectralLow", 0;
	}
	States
	{
	Missile:
		LEAD E 4 A_FaceTarget;
		LEAD F 4 BRIGHT A_ShootGun;
		LEAD E 2 A_SentinelRefire;
		Loop;
	Death:
		LEAD G 5;
		LEAD H 5 A_Scream;
		LEAD IJ 4;
		LEAD K 3;
		LEAD L 3 A_NoBlocking;
		LEAD MNOPQRSTUV 3;
		LEAD W 3 A_SpawnItemEx("AlienSpectre4", 0, 0, 0, 0, 0, random[spectrespawn](0,255)*0.0078125, 0, SXF_NOCHECKPOSITION);
		LEAD X -1;
		Stop;
	}
}


