/*
** zombie.zs
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

// Zombie -------------------------------------------------------------------

class Zombie : StrifeHumanoid
{
	Default
	{
		Health 31;
		Radius 20;
		Height 56;
		PainChance 0;
		+SOLID
		+SHOOTABLE
		+FLOORCLIP
		+CANPASS
		+CANPUSHWALLS
		+ACTIVATEMCROSS
		MinMissileChance 150;
		MaxStepHeight 16;
		MaxDropOffHeight 32;
		Translation 0;
		Tag "$TAG_STRIFEZOMBIE";
		DeathSound "zombie/death";
		CrushPainSound "misc/pcrush";
	}
	States
	{
	Spawn:
		PEAS A 5 A_CheckTerrain;
		Loop;
	Pain:
		AGRD A 5 A_CheckTerrain;
		Loop;
	Death:
		GIBS M 5 A_TossGib;
		GIBS N 5 A_XScream;
		GIBS O 5 A_NoBlocking;
		GIBS PQRST 4 A_TossGib;
		GIBS U 5;
		GIBS V -1;
		Stop;
	}
}


// Zombie Spawner -----------------------------------------------------------

class ZombieSpawner : Actor
{
	Default
	{
		Health 20;
		+SHOOTABLE
		+NOSECTOR
		RenderStyle "None";
		ActiveSound "zombie/spawner";	// Does Strife use this somewhere else?
	}
	States
	{
	Spawn:
		TNT1 A 175 A_SpawnItemEx("Zombie");
		Loop;
	}
}

