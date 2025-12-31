/*
** id24gore.zs
**
** id1 - gore
**
**---------------------------------------------------------------------------
**
** Copyright 1993-2024 id Software LLC, a ZeniMax Media company.
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** converted from DECOHACK
*/

class ID24LargeCorpsePile : Actor
{
	Default
	{
		Radius 40;
		Height 16;
		+SOLID
	}
	States
	{
	Spawn:
		POL7 A -1;
		Stop;
	}
}

class ID24HumanBBQ1 : Actor
{
	Default
	{
		Radius 16;
		Height 16;
		+SOLID
	}
	States
	{
	Spawn:
		HBBQ ABC 5 BRIGHT;
		Loop;
	}
}

class ID24HumanBBQ2 : Actor
{
	Default
	{
		Radius 16;
		Height 16;
		+SOLID
	}
	States
	{
	Spawn:
		HBB2 ABC 5 BRIGHT;
		Loop;
	}
}

class ID24HangingBodyBothLegs : Actor
{
	Default
	{
		Radius 16;
		Height 80;
		+NOGRAVITY
		+SPAWNCEILING
	}
	States
	{
	Spawn:
		GOR6 A -1;
		Stop;
	}
}

class ID24HangingBodyBothLegsSolid : ID24HangingBodyBothLegs
{
	Default
	{
		+SOLID
	}
}

class ID24HangingBodyCrucified : Actor
{
	Default
	{
		Radius 16;
		Height 64;
		+NOGRAVITY
		+SPAWNCEILING
	}
	States
	{
	Spawn:
		GOR7 A -1;
		Stop;
	}
}

class ID24HangingBodyCrucifiedSolid : ID24HangingBodyCrucified
{
	Default
	{
		+SOLID
	}
}

class ID24HangingBodyArmsBound : Actor
{
	Default
	{
		Radius 16;
		Height 72;
		+NOGRAVITY
		+SPAWNCEILING
	}
	States
	{
	Spawn:
		GOR8 A -1;
		Stop;
	}
}

class ID24HangingBodyArmsBoundSolid : ID24HangingBodyArmsBound
{
	Default
	{
		+SOLID
	}
}

class ID24HangingBaronOfHell : Actor
{
	Default
	{
		Radius 16;
		Height 80;
		+NOGRAVITY
		+SPAWNCEILING
	}
	States
	{
	Spawn:
		GORA A -1;
		Stop;
	}
}

class ID24HangingBaronOfHellSolid : ID24HangingBaronOfHell
{
	Default
	{
		+SOLID
	}
}

class ID24HangingChainedBody : Actor
{
	Default
	{
		Radius 16;
		Height 84;
		+NOGRAVITY
		+SPAWNCEILING
	}
	States
	{
	Spawn:
		HDB7 A -1;
		Stop;
	}
}

class ID24HangingChainedBodySolid : ID24HangingChainedBody
{
	Default
	{
		+SOLID
	}
}

class ID24HangingChainedTorso : Actor
{
	Default
	{
		Radius 16;
		Height 52;
		+NOGRAVITY
		+SPAWNCEILING
	}
	States
	{
	Spawn:
		HDB8 A -1;
		Stop;
	}
}

class ID24HangingChainedTorsoSolid : ID24HangingChainedTorso
{
	Default
	{
		+SOLID
	}
}

class ID24SkullPoleTrio : Actor
{
	Default
	{
		Radius 16;
		Height 16;
		+SOLID
	}
	States
	{
	Spawn:
		POLA A -1;
		Stop;
	}
}

class ID24SkullGibs : Actor
{
	Default
	{
		Radius 16;
		Height 16;
	}
	States
	{
	Spawn:
		POB6 A -1;
		Stop;
	}
}
