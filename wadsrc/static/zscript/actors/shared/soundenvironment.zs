/*
** soundenvironment.zs
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

class SoundEnvironment : Actor
{
	default
	{
		+NOSECTOR
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
		+NOTONAUTOMAP
	}
	
	override void PostBeginPlay ()
	{
		Super.PostBeginPlay ();
		if (!bDormant)
		{
			Activate (self);
		}
	}

	override void Activate (Actor activator)
	{
		CurSector.SetEnvironmentID((args[0]<<8) | (args[1]));
	}

	// Deactivate just exists so that you can flag the thing as dormant in an editor
	// and not have it take effect. This is so you can use multiple environments in
	// a single zone, with only one set not-dormant, so you know which one will take
	// effect at the start.
	override void Deactivate (Actor deactivator)
	{
		bDormant = true;
	}
	
}
