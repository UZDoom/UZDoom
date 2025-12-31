/*
** stealthmonsters.zs
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

class StealthArachnotron : Arachnotron
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHBABY";
	}
}

class StealthArchvile : Archvile
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHVILE";
	}
}

class StealthBaron : BaronOfHell
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHBARON";
		HitObituary "$OB_STEALTHBARON";
	}
}

class StealthCacodemon : Cacodemon
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHCACO";
		HitObituary "$OB_STEALTHCACO";
	}
}

class StealthChaingunGuy : ChaingunGuy
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHCHAINGUY";
	}
}

class StealthDemon : Demon
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHDEMON";
		HitObituary "$OB_STEALTHDEMON";
	}
}

class StealthHellKnight : HellKnight
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHKNIGHT";
		HitObituary "$OB_STEALTHKNIGHT";
	}
}

class StealthDoomImp : DoomImp
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHIMP";
		HitObituary "$OB_STEALTHIMP";
	}
}

class StealthFatso : Fatso
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHFATSO";
	}
}

class StealthRevenant : Revenant
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHUNDEAD";
		HitObituary "$OB_STEALTHUNDEAD";
	}
}

class StealthShotgunGuy : ShotgunGuy
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHSHOTGUNGUY";
	}
}

class StealthZombieMan : ZombieMan
{
	Default
	{
		+STEALTH
		RenderStyle "Translucent";
		Alpha 0;
		Obituary "$OB_STEALTHZOMBIE";
	}
}

