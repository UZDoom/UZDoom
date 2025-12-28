/*
** heretickeys.zs
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

Class HereticKey : Key
{
	Default
	{
		+NOTDMATCH
		Radius 20;
		Height 16;
	}
}

// Green key ------------------------------------------------------------

Class KeyGreen : HereticKey
{
	Default
	{
		Inventory.PickupMessage "$TXT_GOTGREENKEY";
		Tag "$TAG_GOTGREENKEY";
		Inventory.Icon "GKEYICON";
	}
	States
	{
	Spawn:
		AKYY ABCDEFGHIJ 3 Bright;
		Loop;
	}
}

// Blue key -----------------------------------------------------------------

Class KeyBlue : HereticKey
{
	Default
	{
		Inventory.PickupMessage "$TXT_GOTBLUEKEY";
		Tag "$TAG_GOTBLUEKEY";
		Inventory.Icon "BKEYICON";
	}
	States
	{
	Spawn:
		BKYY ABCDEFGHIJ 3 Bright;
		Loop;
	}
}

// Yellow key ---------------------------------------------------------------

Class KeyYellow : HereticKey
{
	Default
	{
		Inventory.PickupMessage "$TXT_GOTYELLOWKEY";
		Tag "$TAG_GOTYELLOWKEY";
		Inventory.Icon "YKEYICON";
	}
	States
	{
	Spawn:
		CKYY ABCDEFGHI 3 Bright;
		Loop;
	}
}

 
// --- Blue Key gizmo -----------------------------------------------------------

Class KeyGizmoBlue : Actor
{
	Default
	{
		Radius 16;
		Height 50;
		+SOLID
	}
	States
	{
	Spawn:
		KGZ1 A 1;
		KGZ1 A 1 A_SpawnItemEx("KeyGizmoFloatBlue", 0, 0, 60);
		KGZ1 A -1;
		Stop;
	}
}

Class KeyGizmoFloatBlue : Actor
{
	Default
	{
		Radius 16;
		Height 16;
		+SOLID
		+NOGRAVITY
	}
	States
	{
	Spawn:
		KGZB A -1 Bright;
		Stop;
	}
}

// --- Green Key gizmo -----------------------------------------------------------

Class KeyGizmoGreen : Actor
{
	Default
	{
		Radius 16;
		Height 50;
		+SOLID
	}
	States
	{
	Spawn:
		KGZ1 A 1;
		KGZ1 A 1 A_SpawnItemEx("KeyGizmoFloatGreen", 0, 0, 60);
		KGZ1 A -1;
		Stop;
	}
}

Class KeyGizmoFloatGreen : Actor
{
	Default
	{
		Radius 16;
		Height 16;
		+SOLID
		+NOGRAVITY
	}
	States
	{
	Spawn:
		KGZG A -1 Bright;
		Stop;
	}
}

// --- Yellow Key gizmo -----------------------------------------------------------

Class KeyGizmoYellow : Actor
{
	Default
	{
		Radius 16;
		Height 50;
		+SOLID
	}
	States
	{
	Spawn:
		KGZ1 A 1;
		KGZ1 A 1 A_SpawnItemEx("KeyGizmoFloatYellow", 0, 0, 60);
		KGZ1 A -1;
		Stop;
	}
}

Class KeyGizmoFloatYellow : Actor
{
	Default
	{
		Radius 16;
		Height 16;
		+SOLID
		+NOGRAVITY
	}
	States
	{
	Spawn:
		KGZY A -1 Bright;
		Stop;
	}
}

