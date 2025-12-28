/*
** templar.zs
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

class Templar : Actor
{
	Default
	{
		Health 300;
		Painchance 100;
		Speed 8;
		Radius 20;
		Height 60;
		Mass 500;
		Monster;
		+NOBLOOD
		+SEESDAGGERS
		+NOSPLASHALERT
		MaxdropoffHeight 32;
		MinMissileChance 200;
		SeeSound "templar/sight";
		PainSound "templar/pain";
		DeathSound "templar/death";
		ActiveSound "templar/active";
		CrushPainSound "misc/pcrush";
		Tag "$TAG_TEMPLAR";
		HitObituary "$OB_TEMPLARHIT";
		Obituary "$OB_TEMPLAR";
		DropItem "EnergyPod";
	}

	States
	{
	Spawn:
		PGRD A 5 A_Look2;
		Loop;
		PGRD B 10;
		Loop;
		PGRD C 10;
		Loop;
		PGRD B 10 A_Wander;
		Loop;
	See:
		PGRD AABBCCDD 3 A_Chase;
		Loop;
	Melee:
		PGRD E 8 A_FaceTarget;
		PGRD F 8 A_CustomMeleeAttack(random[ReaverMelee](1,8)*3, "reaver/blade");
		Goto See;
	Missile:
		PGRD G 8 BRIGHT A_FaceTarget;
		PGRD H 8 BRIGHT A_TemplarAttack;
		Goto See;
	Pain:
		PGRD A 2;
		PGRD A 2 A_Pain;
		Goto See;
	Death:
		PGRD I 4 A_TossGib;
		PGRD J 4 A_Scream;
		PGRD K 4 A_TossGib;
		PGRD L 4 A_NoBlocking;
		PGRD MN 4;
		PGRD O 4 A_TossGib;
		PGRD PQRSTUVWXYZ[ 4;
		PGRD \ -1;
		Stop;
	}
	
	void A_TemplarAttack()
	{
		if (target != null)
		{
			A_StartSound ("templar/shoot", CHAN_WEAPON);
			A_FaceTarget ();
			double pitch = AimLineAttack (angle, MISSILERANGE);

			for (int i = 0; i < 10; ++i)
			{
				int damage = random[Templar](1, 4) * 2;
				double ang = angle + random2[Templar]() * (11.25 / 256);
				LineAttack (ang, MISSILERANGE+64., pitch + random2[Templar]() * (7.097 / 256), damage, 'Hitscan', "MaulerPuff");
			}
		}
	}
}

