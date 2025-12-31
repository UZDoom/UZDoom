/*
** chexammo.zs
**
** Renaming and changing messages
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1996-1997 Digital Café
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

// Mini Zorch -----------------------------------------------------------------

class MiniZorchRecharge : Clip
{
	Default
	{
		inventory.pickupmessage "$GOTZORCHRECHARGE";
	}
}

class MiniZorchPack : Clip
{
	Default
	{
		Inventory.PickupMessage "$GOTMINIZORCHPACK";
		Inventory.Amount 50;
	}
	States
	{
		Spawn:
			AMMO A -1;
			Stop;
	}
}

// Large Zorch ----------------------------------------------------------------

class LargeZorchRecharge : Shell
{
	Default
	{
		inventory.pickupmessage "$GOTLARGEZORCHERRECHARGE";
	}
}

class LargeZorchPack : Shell
{
	Default
	{
		Inventory.PickupMessage "$GOTLARGEZORCHERPACK";
		Inventory.Amount 20;
	}
	States
	{
		Spawn:
			SBOX A -1;
			Stop;
	}
}

// Zorch Propulsor ------------------------------------------------------------

class PropulsorZorch : RocketAmmo
{
	Default
	{
		inventory.pickupmessage "$GOTPROPULSORRECHARGE";
	}
}

class PropulsorZorchPack : RocketAmmo
{
	Default
	{
		Inventory.PickupMessage "$GOTPROPULSORPACK";
		Inventory.Amount 5;
	}
	States
	{
		Spawn:
			BROK A -1;
			Stop;
	}
}

// Phasing Zorch --------------------------------------------------------------

class PhasingZorch : Cell
{
	Default
	{
		inventory.pickupmessage "$GOTPHASINGZORCHERRECHARGE";
	}
}

class PhasingZorchPack : Cell
{
	Default
	{
		Inventory.PickupMessage "$GOTPHASINGZORCHERPACK";
		Inventory.Amount 100;
	}
	States
	{
		Spawn:
			CELP A -1;
			Stop;
	}
}
