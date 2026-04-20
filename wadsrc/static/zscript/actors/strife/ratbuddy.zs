/*
** ratbuddy.zs
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
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class RatBuddy : Actor
{
	Default
	{
		Health 5;
		Speed 13;
		Radius 10;
		Height 16;
		+NOBLOOD +FLOORCLIP +CANPASS
		+ISMONSTER +INCOMBAT
		MinMissileChance 150;
		MaxStepHeight 16;
		MaxDropoffHeight 32;
		Tag "$TAG_RATBUDDY";
		SeeSound "rat/sight";
		DeathSound "rat/death";
		ActiveSound "rat/active";
	}
	States
	{
	Spawn:
		RATT A 10 A_Look;
		Loop;
	See:
		RATT AABB 4 A_Chase;
		Loop;
	Melee:
		RATT A 8 A_Wander;
		RATT B 4 A_Wander;
		Goto See;
	Death:
		MEAT Q 700;
		Stop;
	}
}

