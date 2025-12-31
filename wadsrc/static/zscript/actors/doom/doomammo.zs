/*
** doomammo.zs
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

// Clip --------------------------------------------------------------------

class Clip : Ammo
{
	Default
	{
		Inventory.PickupMessage "$GOTCLIP";
		Inventory.Amount 10;
		Inventory.MaxAmount 200;
		Ammo.BackpackAmount 10;
		Ammo.BackpackMaxAmount 400;
		Inventory.Icon "CLIPA0";
		Tag "$AMMO_CLIP";
	}
	States
	{
	Spawn:
		CLIP A -1;
		Stop;
	}
}

// Clip box ----------------------------------------------------------------

class ClipBox : Clip
{
	Default
	{
		Inventory.PickupMessage "$GOTCLIPBOX";
		Inventory.Amount 50;
	}
	States
	{
	Spawn:
		AMMO A -1;
		Stop;
	}
}

// Rocket ------------------------------------------------------------------

class RocketAmmo : Ammo
{
	Default
	{
		Inventory.PickupMessage "$GOTROCKET";
		Inventory.Amount 1;
		Inventory.MaxAmount 50;
		Ammo.BackpackAmount 1;
		Ammo.BackpackMaxAmount 100;
		Inventory.Icon "ROCKA0";
		Tag "$AMMO_ROCKETS";
	}
	States
	{
	Spawn:
		ROCK A -1;
		Stop;
	}
}

// Rocket box --------------------------------------------------------------

class RocketBox : RocketAmmo
{
	Default
	{
		Inventory.PickupMessage "$GOTROCKBOX";
		Inventory.Amount 5;
	}
	States
	{
	Spawn:
		BROK A -1;
		Stop;
	}
}

// Cell --------------------------------------------------------------------

class Cell : Ammo
{
	Default
	{
		Inventory.PickupMessage "$GOTCELL";
		Inventory.Amount 20;
		Inventory.MaxAmount 300;
		Ammo.BackpackAmount 20;
		Ammo.BackpackMaxAmount 600;
		Inventory.Icon "CELLA0";
		Tag "$AMMO_CELLS";
	}
	States
	{
	Spawn:
		CELL A -1;
		Stop;
	}
}

// Cell pack ---------------------------------------------------------------

class CellPack : Cell
{
	Default
	{
		Inventory.PickupMessage "$GOTCELLBOX";
		Inventory.Amount 100;
	}
	States
	{
	Spawn:
		CELP A -1;
		Stop;
	}
}

// Shells ------------------------------------------------------------------

class Shell : Ammo
{
	Default
	{
		Inventory.PickupMessage "$GOTSHELLS";
		Inventory.Amount 4;
		Inventory.MaxAmount 50;
		Ammo.BackpackAmount 4;
		Ammo.BackpackMaxAmount 100;
		Inventory.Icon "SHELA0";
		Tag "$AMMO_SHELLS";
	}
	States
	{
	Spawn:
		SHEL A -1;
		Stop;
	}
}

// Shell box ---------------------------------------------------------------

class ShellBox : Shell
{
	Default
	{
		Inventory.PickupMessage "$GOTSHELLBOX";
		Inventory.Amount 20;
	}
	States
	{
	Spawn:
		SBOX A -1;
		Stop;
	}
}

// Backpack ---------------------------------------------------------------

class Backpack : BackpackItem
{
	Default
	{
		Height 26;
		Inventory.PickupMessage "$GOTBACKPACK";
		Tag "$TAG_BACKPACK";
	}
	States
	{
	Spawn:
		BPAK A -1;
		Stop;
	}
}

