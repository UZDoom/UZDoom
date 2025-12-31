/*
** chexmonsters.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1996-1997 Digital Café
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
// Flemoidus Commonus
//
//===========================================================================

class FlemoidusCommonus : ZombieMan
{
	Default
	{
		DropItem "";
		Obituary "$OB_COMMONUS";
	}
	States
	{
		Missile:
			Stop;
		Melee:
			goto Super::Missile;
	}
}

//===========================================================================
//
// Flemoidus Bipedicus
//
//===========================================================================

class FlemoidusBipedicus : ShotgunGuy
{
	Default
	{
		DropItem "";
		Obituary "$OB_BIPEDICUS";
	}
	States
	{
		Missile:
			Stop;
		Melee:
			goto Super::Missile;
	}
}

//===========================================================================
//
// Flemoidus Bipedicus w/ Armor
//
//===========================================================================

class ArmoredFlemoidusBipedicus : DoomImp
{
	Default
	{
		Obituary "$OB_BIPEDICUS2";
		HitObituary "$OB_BIPEDICUS2";
	}
}

//===========================================================================
//
// Flemoidus Cycloptis Commonus
//
//===========================================================================

class FlemoidusCycloptisCommonus : Demon
{
	Default
	{
		Obituary "$OB_CYCLOPTIS";
	}
}

//===========================================================================
//
// The Flembrane
//
//===========================================================================

class Flembrane : BaronOfHell
{
	Default
	{
		radius 44;
		height 100;
		speed 0;
		Obituary "$OB_FLEMBRANE";
	}
	States
	{
		Missile:
			BOSS EF 3 A_FaceTarget;
			BOSS G 0 A_BruisAttack;
			goto See;
	}
}

//===========================================================================

class ChexSoul : LostSoul
{
	Default
	{
		height 0;
	}
}
