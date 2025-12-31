/*
** strifeweapons.zs
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
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class StrifeWeapon : Weapon
{
	Default
	{
		Weapon.Kickback 100;
	}
}

// Same as the bullet puff for Doom -----------------------------------------

class StrifePuff : Actor
{
	Default
	{
		+NOBLOCKMAP
		+NOGRAVITY
		+ALLOWPARTICLES
		RenderStyle "Translucent";
		Alpha 0.25;
	}

	States
	{
	Spawn:
		POW3 ABCDEFGH 3;
		Stop;
	Crash:
		PUFY A 4 Bright;
		PUFY BCD 4;
		Stop;
	}
}
	

// A spark when you hit something that doesn't bleed ------------------------
// Only used by the dagger.

class StrifeSpark : StrifePuff
{
	Default
	{
		+ZDOOMTRANS
		RenderStyle "Add";
	}
	States
	{
	Crash:
		POW2 ABCD 4;
		Stop;
	}
}

