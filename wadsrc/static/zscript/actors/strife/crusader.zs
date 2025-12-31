/*
** crusader.zs
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

// Crusader -----------------------------------------------------------------

class Crusader : Actor
{
	Default
	{
		Speed 8;
		Radius 40;
		Height 56;
		Mass 400;
		Health 400;
		Painchance 128;
		MissileChanceMult 0.5;
		Monster;
		+FLOORCLIP
		+DONTMORPH
		+INCOMBAT
		+NOICEDEATH
		+NOBLOOD
		MinMissileChance 120;
		MaxDropoffHeight 32;
		DropItem "EnergyPod", 256, 20;
		Tag "$TAG_CRUSADER";
		SeeSound "crusader/sight";
		PainSound "crusader/pain";
		DeathSound "crusader/death";
		ActiveSound "crusader/active";
		Obituary "$OB_CRUSADER";
	}

	States
	{
	Spawn:
		ROB2 Q 10 A_Look;
		Loop;
	See:
		ROB2 AABBCCDD 3 A_Chase;
		Loop;
	Missile:
		ROB2 E 3 Slow A_FaceTarget;
		ROB2 F 2 Slow Bright A_CrusaderChoose;
		ROB2 E 2 Slow Bright A_CrusaderSweepLeft;
		ROB2 F 3 Slow Bright A_CrusaderSweepLeft;
		ROB2 EF 2 Slow Bright A_CrusaderSweepLeft;
		ROB2 EFE 2 Slow Bright A_CrusaderSweepRight;
		ROB2 F 2 Slow A_CrusaderRefire;
		Loop;
	Pain:
		ROB2 D 1 Slow A_Pain;
		Goto See;
	Death:
		ROB2 G 3 A_Scream;
		ROB2 H 5 A_TossGib;
		ROB2 I 4 Bright A_TossGib;
		ROB2 J 4 Bright A_Explode(64, 64, alert:true);
		ROB2 K 4 Bright A_Fall;
		ROB2 L 4 A_Explode(64, 64, alert:true);
		ROB2 MN 4 A_TossGib;
		ROB2 O 4 A_Explode(64, 64, alert:true);
		ROB2 P -1 A_CrusaderDeath;
		Stop;
	}
	
// Crusader -----------------------------------------------------------------

	private bool CrusaderCheckRange ()
	{
		if (reactiontime == 0 && CheckSight (target))
		{
			return Distance2D (target) < 264.;
		}
		return false;
	}

	void A_CrusaderChoose ()
	{
		if (target == null)
			return;

		if (CrusaderCheckRange ())
		{
			A_FaceTarget ();
			angle -= 180./16;
			SpawnMissileZAimed (pos.z + 40, target, "FastFlameMissile");
		}
		else
		{
			if (CheckMissileRange ())
			{
				A_FaceTarget ();
				SpawnMissileZAimed (pos.z + 56, target, "CrusaderMissile");
				angle -= 45./32;
				SpawnMissileZAimed (pos.z + 40, target, "CrusaderMissile");
				angle += 45./16;
				SpawnMissileZAimed (pos.z + 40, target, "CrusaderMissile");
				angle -= 45./16;
				reactiontime += 15;
			}
			SetState (SeeState);
		}
	}

	void A_CrusaderSweepLeft ()
	{
		angle += 90./16;
		Actor misl = SpawnMissileZAimed (pos.z + 48, target, "FastFlameMissile");
		if (misl != null)
		{
			misl.Vel.Z += 1;
		}
	}

	void A_CrusaderSweepRight ()
	{
		angle -= 90./16;
		Actor misl = SpawnMissileZAimed (pos.z + 48, target, "FastFlameMissile");
		if (misl != null)
		{
			misl.Vel.Z += 1;
		}
	}

	void A_CrusaderRefire ()
	{
		if (target == null ||
			target.health <= 0 ||
			!CheckSight (target))
		{
			SetState (SeeState);
		}
	}

	void A_CrusaderDeath ()
	{
		if (CheckBossDeath ())
		{
			Floor_LowerToLowest(667, 8);
		}
	}
}


// Fast Flame Projectile (used by Crusader) ---------------------------------

class FastFlameMissile : FlameMissile
{
	Default
	{
		Mass 50;
		Damage 1;
		Speed 35;
	}
}

// Crusader Missile ---------------------------------------------------------
// This is just like the mini missile the player shoots, except it doesn't
// explode when it dies, and it does slightly less damage for a direct hit.

class CrusaderMissile : Actor
{
	Default
	{
		Speed 20;
		Radius 10;
		Height 14;
		Damage 7;
		Projectile;
		+STRIFEDAMAGE
		MaxStepHeight 4;
		SeeSound "crusader/misl";
		DeathSound "crusader/mislx";
	}
	States
	{
	Spawn:
		MICR A 6 Bright A_RocketInFlight;
		Loop;
	Death:
		SMIS A 0 Bright A_SetRenderStyle(1, STYLE_Normal);
		SMIS A 5 Bright;
		SMIS B 5 Bright;
		SMIS C 4 Bright;
		SMIS DEFG 2 Bright;
		Stop;
	}
}


// Dead Crusader ------------------------------------------------------------

class DeadCrusader : Actor
{
	States
	{
	Spawn:
		ROB2 N 4;
		ROB2 O 4;
		ROB2 P -1;
		Stop;
	}
}

