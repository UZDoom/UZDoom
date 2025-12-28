/*
** fighterboss.zs
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

// Fighter Boss (Zedek) -----------------------------------------------------

class FighterBoss : Actor
{
	Default
	{
		health 800;
		PainChance 50;
		Speed 25;
		Radius 16;
		Height 64;
		Monster;
		+FLOORCLIP
		+TELESTOMP
		+DONTMORPH
		PainSound "PlayerFighterPain";
		DeathSound "PlayerFighterCrazyDeath";
		Obituary "$OB_FBOSS";
		Tag "$FN_FBOSS";
	}

	States
	{
	Spawn:
		PLAY A 2;
		PLAY A 3 A_ClassBossHealth;
		PLAY A 5 A_Look;
		Wait;
	See:
		PLAY ABCD 4 A_FastChase;
		Loop;
	Pain:
		PLAY G 4;
		PLAY G 4 A_Pain;
		Goto See;
	Melee:
	Missile:
		PLAY E 8 A_FaceTarget;
		PLAY F 8 A_FighterAttack;
		Goto See;
	Death:
		PLAY H 6;
		PLAY I 6 A_Scream;
		PLAY JK 6;
		PLAY L 6 A_NoBlocking;
		PLAY M 6;
		PLAY N -1;
		Stop;
	XDeath:
		PLAY O 5 A_Scream;
		PLAY P 5 A_SkullPop;
		PLAY R 5 A_NoBlocking;
		PLAY STUV 5;
		PLAY W -1;
		Stop;
	Ice:
		PLAY X 5 A_FreezeDeath;
		PLAY X 1 A_FreezeDeathChunks;
		Wait;
	Burn:
		FDTH A 5 Bright A_StartSound("PlayerFighterBurnDeath");
		FDTH B 4 Bright;
		FDTH G 5 Bright;
		FDTH H 4 Bright A_Scream;
		FDTH I 5 Bright;
		FDTH J 4 Bright;
		FDTH K 5 Bright;
		FDTH L 4 Bright;
		FDTH M 5 Bright;
		FDTH N 4 Bright;
		FDTH O 5 Bright;
		FDTH P 4 Bright;
		FDTH Q 5 Bright;
		FDTH R 4 Bright;
		FDTH S 5 Bright A_NoBlocking;
		FDTH T 4 Bright;
		FDTH U 5 Bright;
		FDTH V 4 Bright;
		Stop;
	}
	
	//============================================================================
	//
	// A_FighterAttack
	//
	//============================================================================

	void A_FighterAttack()
	{
		if (!target) return;

		SpawnMissileAngle("FSwordMissile", Angle + (45. / 4), 0);
		SpawnMissileAngle("FSwordMissile", Angle + (45. / 8), 0);
		SpawnMissileAngle("FSwordMissile", Angle, 0);
		SpawnMissileAngle("FSwordMissile", Angle - (45. / 8), 0);
		SpawnMissileAngle("FSwordMissile", Angle - (45. / 4), 0);
		A_StartSound ("FighterSwordFire", CHAN_WEAPON);
	}
}
