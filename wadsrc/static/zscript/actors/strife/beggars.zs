/*
** beggars.zs
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

// Base class for the beggars ---------------------------------------------

class Beggar : StrifeHumanoid
{
	Default
	{
		Health 20;
		PainChance 250;
		Speed 3;
		Radius 20;
		Height 56;
		Monster;
		+JUSTHIT
		-COUNTKILL
		+NOSPLASHALERT
		MinMissileChance 150;
		Tag "$TAG_BEGGAR";
		MaxStepHeight 16;
		MaxDropoffHeight 32;
		HitObituary "$OB_BEGGAR";

		AttackSound "beggar/attack";
		PainSound "beggar/pain";
		DeathSound "beggar/death";
	}
	States
	{
	Spawn:
		BEGR A 10 A_Look;
		Loop;
	See:
		BEGR AABBCC 4 A_Wander;
		Loop;
	Melee:
		BEGR D 8;
		BEGR D 8 A_CustomMeleeAttack(2*random[PeasantAttack](1,5)+2);
		BEGR E 1 A_Chase;
		BEGR D 8 A_SentinelRefire;
		Loop;
	Pain:
		BEGR A 3 A_Pain;
		BEGR A 3 A_Chase;
		Goto Melee;
	Death:
		BEGR F 4;
		BEGR G 4 A_Scream;
		BEGR H 4;
		BEGR I 4 A_NoBlocking;
		BEGR JKLM 4;
		BEGR N -1;
		Stop;
	XDeath:
		BEGR F 5 A_TossGib;
		GIBS M 5 A_TossGib;
		GIBS N 5 A_XScream;
		GIBS O 5 A_NoBlocking;
		GIBS PQRST 4 A_TossGib;
		GIBS U 5;
		GIBS V 1400;
		Stop;
	}
}
			

// Beggars -----------------------------------------------------------------

class Beggar1 : Beggar
{
}


class Beggar2 : Beggar
{
}


class Beggar3 : Beggar
{
}


class Beggar4 : Beggar
{
}


class Beggar5 : Beggar
{
}
