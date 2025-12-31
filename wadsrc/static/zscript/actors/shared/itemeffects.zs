/*
** itemeffects.zs
**
** shown for respawning Doom and Strife items
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

class ItemFog : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOGRAVITY
	}
	States
	{
	Spawn:
		IFOG ABABCDE 6 BRIGHT;
		Stop;
	}
}

// Pickup flash -------------------------------------------------------------

class PickupFlash : Actor
{
	default
	{
		+NOGRAVITY
	}
	States
	{
	Spawn:
		ACLO DCDCBCBABA 3;
		Stop;
	}
}

