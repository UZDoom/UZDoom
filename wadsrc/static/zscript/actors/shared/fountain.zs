/*
** fountain.zs
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

class ParticleFountain : Actor
{
	enum EColor
	{
		REDFOUNTAIN		= 1,
		GREENFOUNTAIN	= 2,
		BLUEFOUNTAIN	= 3,
		YELLOWFOUNTAIN	= 4,
		PURPLEFOUNTAIN	= 5,
		BLACKFOUNTAIN	= 6,
		WHITEFOUNTAIN	= 7
	}

	default
	{
		Height 0;
		+NOBLOCKMAP
		+NOGRAVITY
		+INVISIBLE
	}
	
	override void PostBeginPlay ()
	{
		Super.PostBeginPlay ();
		if (!(SpawnFlags & MTF_DORMANT))
			Activate (null);
	}

	override void Activate (Actor activator)
	{
		Super.Activate (activator);
		fountaincolor = health;
	}

	override void Deactivate (Actor activator)
	{
		Super.Deactivate (activator);
		fountaincolor = 0;
	}
}

class RedParticleFountain : ParticleFountain
{
	default
	{
		Health REDFOUNTAIN;
	}
}

class GreenParticleFountain : ParticleFountain
{
	default
	{
		Health GREENFOUNTAIN;
	}
}

class BlueParticleFountain : ParticleFountain
{
	default
	{
		Health BLUEFOUNTAIN;
	}
}

class YellowParticleFountain : ParticleFountain
{
	default
	{
		Health YELLOWFOUNTAIN;
	}
}

class PurpleParticleFountain : ParticleFountain
{
	default
	{
		Health PURPLEFOUNTAIN;
	}
}

class BlackParticleFountain : ParticleFountain
{
	default
	{
		Health BLACKFOUNTAIN;
	}
}

class WhiteParticleFountain : ParticleFountain
{
	default
	{
		Health WHITEFOUNTAIN;
	}
}
