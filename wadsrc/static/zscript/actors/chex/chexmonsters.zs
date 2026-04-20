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
** SPDX-License-Identifier: GPL-3.0-or-later
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
