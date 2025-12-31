/*
** setcolor.zs
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

class ColorSetter : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
		RenderStyle "None";
	}
	
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		CurSector.SetColor(color(args[0], args[1], args[2]), args[3]);
		Destroy();
	}
	
}


class FadeSetter : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
		RenderStyle "None";
	}
	
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		CurSector.SetFade(color(args[0], args[1], args[2]));
		Destroy();
	}

	
}
