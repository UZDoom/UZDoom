/*
** clericboss.zs
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

// Cleric Boss (Traductus) --------------------------------------------------

class ClericBoss : Actor
{
	Default
	{
		Health 800;
		PainChance 50;
		Speed 25;
		Radius 16;
		Height 64;
		Monster;
		+FLOORCLIP +TELESTOMP
		+DONTMORPH
		PainSound "PlayerClericPain";
		DeathSound "PlayerClericCrazyDeath";
		Obituary "$OB_CBOSS";
		Tag "$FN_CBOSS";
	}

	States
	{
	Spawn:
		CLER A 2;
		CLER A 3 A_ClassBossHealth;
		CLER A 5 A_Look;
		Wait;
	See:
		CLER ABCD 4 A_FastChase;
		Loop;
	Pain:
		CLER H 4;
		CLER H 4 A_Pain;
		Goto See;
	Melee:
	Missile:
		CLER EF 8 A_FaceTarget;
		CLER G 10 A_ClericAttack;
		Goto See;
	Death:
		CLER I 6;
		CLER K 6 A_Scream;
		CLER LL 6;
		CLER M 6 A_NoBlocking;
		CLER NOP 6;
		CLER Q -1;
		Stop;
	XDeath:
		CLER R 5 A_Scream;
		CLER S 5;
		CLER T 5 A_NoBlocking;
		CLER UVWXYZ 5;
		CLER [ -1;
		Stop;
	Ice:
		CLER \ 5 A_FreezeDeath;
		CLER \ 1 A_FreezeDeathChunks;
		Wait;
	Burn:
		CLER C 5 Bright A_StartSound("PlayerClericBurnDeath");
		FDTH D 4 Bright ;
		FDTH G 5 Bright ;
		FDTH H 4 Bright A_Scream;
		FDTH I 5 Bright ;
		FDTH J 4 Bright ;
		FDTH K 5 Bright ;
		FDTH L 4 Bright ;
		FDTH M 5 Bright ;
		FDTH N 4 Bright ;
		FDTH O 5 Bright ;
		FDTH P 4 Bright ;
		FDTH Q 5 Bright ;
		FDTH R 4 Bright ;
		FDTH S 5 Bright A_NoBlocking;
		FDTH T 4 Bright ;
		FDTH U 5 Bright ;
		FDTH V 4 Bright ;
		Stop;
	}
		
	//============================================================================
	//
	// A_ClericAttack
	//
	//============================================================================

	void A_ClericAttack()
	{
		if (!target) return;

		Actor missile = SpawnMissileZ (pos.z + 40., target, "HolyMissile");
		if (missile != null) missile.tracer = null;	// No initial target
		A_StartSound ("HolySymbolFire", CHAN_WEAPON);
	}
}
