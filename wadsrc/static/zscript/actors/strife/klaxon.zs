/*
** klaxon.zs
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

// Klaxon Warning Light -----------------------------------------------------

class KlaxonWarningLight : Actor
{
	Default
	{
		ReactionTime 60;
		Radius 5;
		+NOBLOCKMAP +AMBUSH
		+SPAWNCEILING +NOGRAVITY
		+FIXMAPTHINGPOS +NOSPLASHALERT
		+SYNCHRONIZED
	}
	States
	{
	Spawn:
		KLAX A 5 A_TurretLook;
		Loop;
	See:
		KLAX B 6 A_KlaxonBlare;
		KLAX C 60;
		Loop;
	}

}

// CeilingTurret ------------------------------------------------------------

class CeilingTurret : Actor
{
	Default
	{
		Health 125;
		Speed 0;
		Painchance 0;
		Mass 10000000;
		Monster;
		-SOLID
		-CANPASS
		+AMBUSH
		+SPAWNCEILING
		+NOGRAVITY
		+NOBLOOD
		+NOSPLASHALERT
		+DONTFALL
		MinMissileChance 150;
		Tag "$TAG_CEILINGTURRET";
		Obituary "$OB_TURRET";
		DeathSound "turret/death";
	}
	States
	{
	Spawn:
		TURT A 5 A_TurretLook;
		Loop;
	See:
		TURT A 2 A_Chase;
		Loop;
	Missile:
	Pain:
		TURT B 4 Slow A_ShootGun;
		TURT D 3 Slow A_SentinelRefire;
		TURT A 4 A_SentinelRefire;
		Loop;
	Death:
		BALL A 6 Bright A_Scream;
		BALL BCDE 6 Bright;
		TURT C -1;
		Stop;
	}
}


extend class Actor
{
	void A_TurretLook()
	{
		if (bInConversation)
			return;

		threshold = 0;
		Actor targ = LastHeard;
		if (targ != NULL && targ.health > 0 && targ.bShootable && !IsFriend(targ))
		{
			target = targ;
			if (bAmbush && !CheckSight (targ))
			{
				return;
			}
			if (SeeSound != 0)
			{
				A_StartSound (SeeSound, CHAN_VOICE);
			}
			LastHeard = NULL;
			threshold = 10;
			SetState (SeeState);
		}
	}

	void A_KlaxonBlare()
	{
		if (--reactiontime < 0)
		{
			target = NULL;
			reactiontime = Default.reactiontime;
			A_TurretLook();
			if (target == NULL)
			{
				SetIdle();
			}
			else
			{
				reactiontime = 50;
			}
		}
		if (reactiontime == 2)
		{
			// [RH] Unalert monsters near the alarm and not just those in the same sector as it.
			SoundAlert (NULL, false);
		}
		else if (reactiontime > 50)
		{
			A_StartSound ("misc/alarm", CHAN_VOICE);
		}
	}

}
