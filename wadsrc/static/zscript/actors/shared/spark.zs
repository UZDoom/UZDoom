/*
** spark.zs
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

class Spark : Actor
{
	default
	{
		+NOSECTOR
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
	}
	
	override void Activate (Actor activator)
	{
		Super.Activate (activator);
		DrawSplash (args[0] ? args[0] : 32, Angle, 1);
		A_StartSound ("world/spark", CHAN_AUTO, CHANF_DEFAULT, 1, ATTN_STATIC);
	}
	
}
