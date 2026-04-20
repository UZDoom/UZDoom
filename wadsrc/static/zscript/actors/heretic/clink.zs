/*
** clink.zs
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
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class Clink : Actor
{
	Default
	{
		Health 150;
		Radius 20;
		Height 64;
		Mass 75;
		Speed 14;
		Painchance 32;
		Monster;
		+NOBLOOD
		+FLOORCLIP
		SeeSound "clink/sight";
		AttackSound "clink/attack";
		PainSound "clink/pain";
		DeathSound "clink/death";
		ActiveSound "clink/active";
		Obituary "$OB_CLINK";
		Tag "$FN_CLINK";
		DropItem "SkullRodAmmo", 84, 20;
	}
	States
	{
	Spawn:
		CLNK AB 10 A_Look;
		Loop;
	See:
		CLNK ABCD 3 A_Chase;
		Loop;
	Melee:
		CLNK E 5 A_FaceTarget;
		CLNK F 4 A_FaceTarget;
		CLNK G 7 A_CustomMeleeAttack(random[ClinkAttack](3,9), "clink/attack", "clink/attack");
		Goto See;
	Pain:
		CLNK H 3;
		CLNK H 3 A_Pain;
		Goto See;
	Death:
		CLNK IJ 6;
		CLNK K 5 A_Scream;
		CLNK L 5 A_NoBlocking;
		CLNK MN 5;
		CLNK O -1;
		Stop;
	}
}

