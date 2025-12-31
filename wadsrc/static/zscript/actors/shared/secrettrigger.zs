/*
** secrettrigger.zs
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

class SecretTrigger : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOSECTOR
		+NOGRAVITY
		+DONTSPLASH
		+NOTONAUTOMAP
	}

	override void PostBeginPlay ()
	{
		Super.PostBeginPlay ();
		level.total_secrets++;
	}

	override void Activate (Actor activator)
	{
		Level.GiveSecret(activator, args[0] <= 1, (args[0] == 0 || args[0] == 2));
		Destroy ();
	}

	
}

