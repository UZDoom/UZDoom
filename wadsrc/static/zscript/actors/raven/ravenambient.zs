/*
** ravenambient.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
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

// Wind ---------------------------------------------------------------------

class SoundWind : Actor
{
	Default
	{
		+NOBLOCKMAP
		+NOSECTOR
		+DONTSPLASH
	}
	States
	{
	Spawn:
		TNT1 A 2 A_StartSound("world/wind", CHAN_6, CHANF_LOOPING);
		Loop;
	}
}

class SoundWindHexen : SoundWind
{
}


// Waterfall ----------------------------------------------------------------

class SoundWaterfall : Actor
{
	Default
	{
		+NOBLOCKMAP
		+NOSECTOR
		+DONTSPLASH
	}
	States
	{
	Spawn:
		TNT1 A 2 A_StartSound("world/waterfall", CHAN_6, CHANF_LOOPING);
		Loop;
	}
}
