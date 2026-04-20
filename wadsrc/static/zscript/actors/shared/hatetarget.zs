/*
** hatetarget.zs
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
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

// Hate Target --------------------------------------------------------------

class HateTarget : Actor
{
	default
	{
		Radius 20;
		Height 56;
		+SHOOTABLE
		+NOGRAVITY
		+NOBLOOD
		+DONTSPLASH
		Mass 0x7fffffff;
	}
	States
	{
	Spawn:
		TNT1 A -1;
	}
	
	override void BeginPlay()
	{
		Super.BeginPlay();
		if (SpawnAngle != 0)
		{	// Each degree translates into 10 units of health
			health = SpawnAngle * 10;
		}
		else
		{
			special2 = 1;
			health = 1000001;
		}
	}

	override int TakeSpecialDamage(Actor inflictor, Actor source, int damage, Name damagetype, int flags, double angle)
	{
		if (special2 != 0)
		{
			return 0;
		}
		else
		{
			return damage;
		}
	}

	
}
