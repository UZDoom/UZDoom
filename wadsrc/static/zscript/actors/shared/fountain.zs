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
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
