/*
** strifebishop.zs
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

// Bishop -------------------------------------------------------------------

class StrifeBishop : Actor
{
	Default
	{
		Health 500;
		Painchance 128;
		Speed 8;
		Radius 40;
		Height 56;
		Mass 500;
		Monster;
		+NOBLOOD
		+NOTDMATCH
		+FLOORCLIP
		+INCOMBAT
		+NOICEDEATH
		+NEVERRESPAWN
		DamageFactor "Fire", 0.5;
		MinMissileChance 150;
		MaxDropoffHeight 32;
		Tag "$TAG_BISHOP";
		SeeSound "bishop/sight";
		PainSound "bishop/pain";
		DeathSound "bishop/death";
		ActiveSound "bishop/active";
		DropItem "CrateOfMissiles", 256, 20;
		Obituary "$OB_STFBISHOP";
	}
	States
	{
	Spawn:
		MLDR A 10 A_Look;
		Loop;
	See:
		MLDR AABBCCDD 3 A_Chase;
		Loop;
	Missile:
		MLDR E 3 A_FaceTarget;
		MLDR F 2 Bright A_SpawnProjectile("BishopMissile", 64, 0, 0, CMF_AIMOFFSET);
		Goto See;
	Pain:
		MLDR D 1 A_Pain;
		Goto See;
	Death:
		MLDR G 3 Bright;
		MLDR H 5 Bright A_Scream;
		MLDR I 4 Bright A_TossGib;
		MLDR J 4 Bright A_Explode(64, 64, alert:true);
		MLDR KL 3 Bright;
		MLDR M 4 Bright A_NoBlocking;
		MLDR N 4 Bright;
		MLDR O 4 Bright A_TossGib;
		MLDR P 4 Bright;
		MLDR Q 4 Bright A_TossGib;
		MLDR R 4 Bright;
		MLDR S 4 Bright A_TossGib;
		MLDR T 4 Bright;
		MLDR U 4 Bright A_TossGib;
		MLDR V 4 Bright A_SpawnItemEx("AlienSpectre2", 0, 0, 0, 0, 0, random[spectrespawn](0,255)*0.0078125, 0, SXF_NOCHECKPOSITION);
		Stop;
	}
}


// The Bishop's missile -----------------------------------------------------

class BishopMissile : Actor
{
	Default
	{
		Speed 20;
		Radius 10;
		Height 14;
		Damage 10;
		Projectile;
		+SEEKERMISSILE
		+STRIFEDAMAGE
		MaxStepHeight 4;
		SeeSound "bishop/misl";
		DeathSound "bishop/mislx";
	}
	States
	{
	Spawn:
		MISS A 4 Bright A_RocketInFlight;
		MISS B 3 Bright A_Tracer2;
		Loop;
	Death:
		SMIS A 0 Bright A_SetRenderStyle(1, STYLE_Normal);
		SMIS A 5 Bright A_Explode(64, 64, alert:true);
		SMIS B 5 Bright;
		SMIS C 4 Bright;
		SMIS DEFG 2 Bright;
		Stop;
	}
}

