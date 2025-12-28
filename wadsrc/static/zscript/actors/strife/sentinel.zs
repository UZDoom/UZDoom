/*
** sentinel.zs
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

// Sentinel -----------------------------------------------------------------

class Sentinel : Actor
{
	Default
	{
		Health 100;
		Painchance 255;
		Speed 7;
		Radius 23;
		Height 53;
		Mass 300;
		MissileChanceMult 0.5;
		Monster;
		+SPAWNCEILING
		+NOGRAVITY
		+DROPOFF
		+NOBLOOD
		+NOBLOCKMONST
		+INCOMBAT
		+LOOKALLAROUND
		+NEVERRESPAWN
		MinMissileChance 150;
		Tag "$TAG_SENTINEL";
		SeeSound "sentinel/sight";
		DeathSound "sentinel/death";
		ActiveSound "sentinel/active";
		Obituary "$OB_SENTINEL";
	}

	States
	{
	Spawn:
		SEWR A 10 A_Look;
		Loop;
	See:
		SEWR A 6 A_SentinelBob;
		SEWR A 6 A_Chase;
		Loop;
	Missile:
		SEWR B 4 A_FaceTarget;
		SEWR C 8 Bright A_SentinelAttack;
		SEWR C 4 Bright A_SentinelRefire;
		Goto Missile+1;
	Pain:
		SEWR D 5 A_Pain;
		Goto Missile+2;
	Death:
		SEWR D 7 A_Fall;
		SEWR E 8 Bright A_TossGib;
		SEWR F 5 Bright A_Scream;
		SEWR GH 4 Bright A_TossGib;
		SEWR I 4;
		SEWR J 5;
		Stop;
	}
	
	void A_SentinelAttack ()
	{
		// [BB] Without a target the P_SpawnMissileZAimed call will crash.
		if (!target)
		{
			return;
		}

		Actor missile = SpawnMissileZAimed (pos.z + 32, target, "SentinelFX2");

		if (missile != NULL && (missile.Vel.X != 0 || missile.Vel.Y != 0))
		{
			for (int i = 8; i > 1; --i)
			{
				Actor trail = Spawn("SentinelFX1", Vec3Angle(missile.radius*i, missile.angle, 32 + missile.Vel.Z / 4 * i), ALLOW_REPLACE);
				if (trail != NULL)
				{
					trail.target = self;
					trail.Vel = missile.Vel;
					trail.CheckMissileSpawn (radius);
				}
			}
			missile.AddZ(missile.Vel.Z / 4);
		}
	}

	
}

// Sentinel FX 1 ------------------------------------------------------------

class SentinelFX1 : Actor
{
	Default
	{
		Speed 40;
		Radius 10;
		Height 8;
		Damage 0;
		DamageType "Disintegrate";
		Projectile;
		+STRIFEDAMAGE
		+ZDOOMTRANS
		MaxStepHeight 4;
		RenderStyle "Add";
	}
	States
	{
	Spawn:
		SHT1 AB 4;
		Loop;
	Death:
		POW1 J 4;
		Stop;
	}
}

// Sentinel FX 2 ------------------------------------------------------------

class SentinelFX2 : SentinelFX1
{
	Default
	{
	SeeSound "sentinel/plasma";
	Damage 1;
	}
	States
	{
	Death:
		POW1 FGHI 4;
		Goto Super::Death;
	}
}


extend class Actor
{
	// These are used elsewhere, too.
	void A_SentinelBob()
	{
		if (bInFloat)
		{
			Vel.Z = 0;
			return;
		}
		if (threshold != 0)
			return;

		double maxz =  ceilingz - Height - 16;
		double minz = floorz + 96;
		if (minz > maxz)
		{
			minz = maxz;
		}
		if (minz < pos.z)
		{
			Vel.Z -= 1;
		}
		else
		{
			Vel.Z += 1;
		}
		reactiontime = (minz >= pos.z) ? 4 : 0;
	}

	void A_SentinelRefire()
	{
		A_FaceTarget ();
		if (HitFriend())
		{
			SetState(SeeState);
			return;
		}

		if (random[SentinelRefire]() >= 30)
		{
			if (target == NULL ||
				target.health <= 0 ||
				!CheckSight (target, SF_SEEPASTBLOCKEVERYTHING|SF_SEEPASTSHOOTABLELINES) ||
				(MissileState == NULL && !CheckMeleeRange()) ||
				random[SentinelRefire]() < 40)
			{
				SetState (SeeState);
			}
		}
	}
}
