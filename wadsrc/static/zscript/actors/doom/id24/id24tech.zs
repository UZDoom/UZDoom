/*
** id24tech.zs
**
** id1 - tech
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

class ID24OfficeChair : Actor
{
	Default
	{
		Radius 12;
		Height 16;
		+SOLID
	}
	States
	{
	Spawn:
		CHR1 A -1;
		Stop;
	}
}

class ID24OfficeLamp : Actor
{
	Default
	{
		Radius 12;
		Height 52;
		Health 20;
		Mass 65536;
		DeathSound "world/officelamp";
		+SOLID
		+SHOOTABLE
		+NOBLOOD
		+DONTGIB
		+NOICEDEATH
	}
	States
	{
	Spawn:
		LAMP A -1 BRIGHT;
		Stop;
	Death:
		LAMP B 3 BRIGHT A_Scream;
		LAMP B 3 BRIGHT A_Fall;
	DeathLoop:
		LAMP CD 5 BRIGHT;
		Loop;
	}
}

class ID24CeilingLamp : Actor
{
	Default
	{
		Radius 12;
		Height 32;
		+SPAWNCEILING
		+NOGRAVITY
	}
	States
	{
	Spawn:
		TLP6 A -1 BRIGHT;
		Stop;
	}
}

class ID24CandelabraShort : Actor
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
		CBR2 A -1 BRIGHT;
		Stop;
	}
}
