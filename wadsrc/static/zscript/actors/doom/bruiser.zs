/*
** bruiser.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
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

//===========================================================================
//
// Baron of Hell
//
//===========================================================================
class BaronOfHell : Actor
{
	Default
	{
		Health 1000;
		Radius 24;
		Height 64;
		Mass 1000;
		Speed 8;
		PainChance 50;
		Monster;
		+FLOORCLIP
		+BOSSDEATH
		+E1M8BOSS
		SeeSound "baron/sight";
		PainSound "baron/pain";
		DeathSound "baron/death";
		ActiveSound "baron/active";
		Obituary "$OB_BARON";
		HitObituary "$OB_BARONHIT";
		Tag "$FN_BARON";
	}
	States
	{
	Spawn:
		BOSS AB 10 A_Look ;
		Loop;
	See:
		BOSS AABBCCDD 3 A_Chase;
		Loop;
	Melee:
	Missile:
		BOSS EF 8 A_FaceTarget;
		BOSS G 8 A_BruisAttack;
		Goto See;
	Pain:
		BOSS H  2;
		BOSS H  2 A_Pain;
		Goto See;
	Death:
		BOSS I  8;
		BOSS J  8 A_Scream;
		BOSS K  8;
		BOSS L  8 A_NoBlocking;
		BOSS MN 8;
		BOSS O -1 A_BossDeath;
		Stop;
	Raise:
		BOSS O 8;
		BOSS NMLKJI  8;
		Goto See;
	}
}

//===========================================================================
//
// Hell Knight
//
//===========================================================================
class HellKnight : BaronOfHell
{
	Default
	{
		Health 500;
		-BOSSDEATH;
		SeeSound "knight/sight";
		ActiveSound "knight/active";
		PainSound "knight/pain";
		DeathSound "knight/death";
		HitObituary "$OB_KNIGHTHIT";
		Obituary "$OB_KNIGHT";
		Tag "$FN_HELL";
	}
	States
	{
	Spawn:
		BOS2 AB 10 A_Look;
		Loop;
	See:
		BOS2 AABBCCDD 3 A_Chase;
		Loop;
	Melee:
	Missile:
		BOS2 EF 8 A_FaceTarget;
		BOS2 G 8 A_BruisAttack;
		Goto See;
	Pain:
		BOS2 H  2;
		BOS2 H  2 A_Pain;
		Goto See;
	Death:
		BOS2 I  8;
		BOS2 J  8 A_Scream;
		BOS2 K  8;
		BOS2 L  8 A_NoBlocking;
		BOS2 MN 8;
		BOS2 O -1;
		Stop;
	Raise:
		BOS2 O 8;
		BOS2 NMLKJI  8;
		Goto See;
	}
}

//===========================================================================
//
// Baron slime ball
//
//===========================================================================
class BaronBall : Actor
{
	Default
	{
		Radius 6;
		Height 16;
		Speed 15;
		FastSpeed 20;
		Damage 8;
		Projectile ;
		+RANDOMIZE
		+ZDOOMTRANS
		RenderStyle "Add";
		Alpha 1;
		SeeSound "baron/attack";
		DeathSound "baron/shotx";
		Decal "BaronScorch";
	}
	States
	{
	Spawn:
		BAL7 AB 4 BRIGHT;
		Loop;
	Death:
		BAL7 CDE 6 BRIGHT;
		Stop;
	}
}

//===========================================================================
//
// Code (must be attached to Actor)
//
//===========================================================================

extend class Actor
{
	void A_BruisAttack()
	{
		let targ = target;
		if (targ)
		{
			if (CheckMeleeRange())
			{
				int damage = random[pr_bruisattack](1, 8) * 10;
				A_StartSound ("baron/melee", CHAN_WEAPON);
				int newdam = target.DamageMobj (self, self, damage, "Melee");
				targ.TraceBleed (newdam > 0 ? newdam : damage, self);
			}
			else
			{
				// launch a missile
				SpawnMissile (target, "BaronBall");
			}
		}
	}
}
