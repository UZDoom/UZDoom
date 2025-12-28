/*
** strifehumanoid.zs
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

// Humanoid Base Class ------------------------------------------------------


class StrifeHumanoid : Actor
{
	Default
	{
		MaxStepHeight 16;
		MaxDropoffHeight 32;
		CrushPainSound "misc/pcrush";
	}
	States
	{
	Burn:
		BURN A 3 Bright Light("PhFire_FX1") A_StartSound("human/imonfire", CHAN_VOICE);
		BURN B 3 Bright Light("PhFire_FX2") A_DropFire;
		BURN C 3 Bright Light("PhFire_FX3") A_Wander;
		BURN D 3 Bright Light("PhFire_FX4") A_NoBlocking;
		BURN E 5 Bright Light("PhFire_FX5") A_DropFire;
		BURN F 5 Bright Light("PhFire_FX6") A_Wander;
		BURN G 5 Bright Light("PhFire_FX7") A_Wander;
		BURN H 5 Bright Light("PhFire_FX6") A_Wander;
		BURN I 5 Bright Light("PhFire_FX5") A_DropFire;
		BURN J 5 Bright Light("PhFire_FX4") A_Wander;
		BURN K 5 Bright Light("PhFire_FX3") A_Wander;
		BURN L 5 Bright Light("PhFire_FX2") A_Wander;
		BURN M 5 Bright Light("PhFire_FX1") A_DropFire;
		BURN N 5 Bright Light("PhFire_FX2");
		BURN O 5 Bright Light("PhFire_FX3");
		BURN P 5 Bright Light("PhFire_FX4");
		BURN Q 5 Bright Light("PhFire_FX5");
		BURN P 5 Bright Light("PhFire_FX4");
		BURN Q 5 Bright Light("PhFire_FX5");
		BURN R 7 Bright Light("PhFire_FX8");
		BURN S 7 Bright Light("PhFire_FX9");
		BURN T 7 Bright Light("PhFire_FX10");
		BURN U 7 Bright Light("PhFire_FX11");
		BURN V -1;
		Stop;
	Disintegrate:
		DISR A 5 A_StartSound("misc/disruptordeath", CHAN_VOICE);
		DISR BC 5;
		DISR D 5 A_NoBlocking;
		DISR EF 5;
		DISR GHIJ 4;
		MEAT D 700;
		Stop;
	}
}


// Fire Droplet -------------------------------------------------------------

class FireDroplet : Actor
{
	Default
	{
		+NOBLOCKMAP
		+NOCLIP
	}
	States
	{
	Spawn:
		FFOT ABCD 9 Bright;
		Stop;
	}
}
