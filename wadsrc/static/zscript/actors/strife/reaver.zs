/*
** reaver.zs
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

class Reaver : Actor
{
	Default
	{
		Health 150;
		Painchance 128;
		Speed 12;
		Radius 20;
		Height 60;
		Monster;
		+NOBLOOD
		+INCOMBAT
		MinMissileChance 150;
		MaxDropoffHeight 32;
		Mass 500;
		Tag "$TAG_REAVER";
		SeeSound "reaver/sight";
		PainSound "reaver/pain";
		DeathSound "reaver/death";
		ActiveSound "reaver/active";
		HitObituary "$OB_REAVERHIT";
		Obituary "$OB_REAVER";
	}
	
	States
	{
	Spawn:
		ROB1 A 10 A_Look;
		Loop;
	See:
		ROB1 BBCCDDEE 3 A_Chase;
		Loop;
	Melee:
		ROB1 H 6 Slow A_FaceTarget;
		ROB1 I 8 Slow A_CustomMeleeAttack(random[ReaverMelee](1,8)*3, "reaver/blade");
		ROB1 H 6 Slow;
		Goto See;
	Missile:
		ROB1 F 8 Slow A_FaceTarget;
		ROB1 G 11 Slow BRIGHT A_ReaverRanged;
		Goto See;
	Pain:
		ROB1 A 2 Slow;
		ROB1 A 2 A_Pain;
		Goto See;
	Death:
		ROB1 J 6;
		ROB1 K 6 A_Scream;
		ROB1 L 5;
		ROB1 M 5 A_NoBlocking;
		ROB1 NOP 5;
		ROB1 Q 6 A_Explode(32, 32, alert:true);
		ROB1 R -1;
		Stop;
	XDeath:
		ROB1 L 5 A_TossGib;
		ROB1 M 5 A_Scream;
		ROB1 N 5 A_TossGib;
		ROB1 O 5 A_NoBlocking;
		ROB1 P 5 A_TossGib;
		Goto Death+7;
	}

}
		
extend class Actor
{
	// The Inquisitor also uses this function
	
	void A_ReaverRanged ()
	{
		if (target != null)
		{
			A_FaceTarget ();
			A_StartSound ("reaver/attack", CHAN_WEAPON);
			double bangle = Angle;
			double pitch = AimLineAttack (bangle, MISSILERANGE);

			for (int i = 0; i < 3; ++i)
			{
				double ang = bangle + Random2[ReaverAttack]() * (22.5 / 256);
				int damage = random[ReaverAttack](1, 8) * 3;
				LineAttack (ang, MISSILERANGE, pitch, damage, 'Hitscan', "StrifePuff");
			}
		}
	}

}
