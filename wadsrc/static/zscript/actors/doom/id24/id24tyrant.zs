/*
** id24tyrant.zs
**
** id1 - tyrant
**
**---------------------------------------------------------------------------
**
** Copyright 1993-2024 id Software LLC, a ZeniMax Media company.
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** converted from DECOHACK
*/

class ID24Tyrant : Cyberdemon
{
	Default
	{
		Health 1000;
		ProjectileGroup 2;
		SplashGroup 2;
		Monster;
		+BOSS
		SeeSound "monsters/tyrant/sight";
		PainSound "monsters/tyrant/pain";
		DeathSound "monsters/tyrant/death";
		ActiveSound "monsters/tyrant/active";
		Obituary "$ID24_OB_TYRANT";
		Tag "$ID24_CC_TYRANT";
	}
	States
	{
	Spawn:
		CYB2 AB 10 A_Look;
		Loop;
	See:
		CYB2 A 3 A_Hoof;
		CYB2 ABBCC 3 A_Chase;
		CYB2 D 0 A_StartSound("monsters/tyrant/walk");
		CYB2 DD 3 A_Chase;
		Loop;
	Missile:
		CYB2 E 6 A_FaceTarget;
		CYB2 F 12 BRIGHT A_CyberAttack;
		CYB2 E 12 A_FaceTarget;
		CYB2 F 12 BRIGHT A_CyberAttack;
		CYB2 E 12 A_FaceTarget;
		CYB2 F 12 BRIGHT A_CyberAttack;
		Goto See;
	Pain:
		CYB2 G 10 A_Pain;
		goto See;
	Death:
		CYB2 H 10;
		CYB2 I 10 A_Scream;
		CYB2 JKL 10;
		CYB2 M 10 A_Fall;
		CYB2 NO 10;
		CYB2 P 30;
		CYB2 P -1 A_BossDeath;
		Stop;
	}
}

// "boss" variants -- these are simply alternative
// actors for use with UMAPINFO's "bossaction"
// field to make the final map's staged fights
// work nicely. they're functionally identical
// to the main actor in every other way.

// [XA] well, now they also have a 60-second respawn
// minimum instead of the default 12, so Nightmare
// sucks a bit less ;)

// (this id24 feature isn't implemented yet!)

class ID24TyrantBoss1 : ID24Tyrant
{
	Default
	{
		// RespawnTics 2100;
		// RespawnDice 64;
	}
}

class ID24TyrantBoss2 : ID24Tyrant
{
	Default
	{
		// RespawnTics 2100;
		// RespawnDice 64;
	}
}
