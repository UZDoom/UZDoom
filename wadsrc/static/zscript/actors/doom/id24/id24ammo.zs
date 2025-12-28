/*
** id24ammo.zs
**
** id1 - ammo
**
**---------------------------------------------------------------------------
**
** Copyright 1993-2024 id Software LLC, a ZeniMax Media company.
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
** converted from DECOHACK and from id24data.cpp
*/

class ID24Fuel : Ammo
{
	Default
	{
		Inventory.PickupMessage "$ID24_GOTFUELCAN";
		Inventory.Amount 10;
		Inventory.MaxAmount 150;
		Ammo.BackpackAmount 10;
		Ammo.BackpackMaxAmount 300;
		Inventory.Icon "FTNKA0";
		Tag "$AMMO_ID24FUEL";
	}
	States
	{
	Spawn:
		FCPU A -1;
		Stop;
	}
}

class ID24FuelTank : ID24Fuel
{
	Default
	{
		Inventory.PickupMessage "$ID24_GOTFUELTANK";
		Inventory.Amount 50;
		Ammo.DropAmmoFactorMultiplier 0.8; // tank has 20 drop amount, not 25, so multiply the factor by a further 0.8
										   // -- for custom dehacked ammo this can be calculated based on the drop amount and the default factor
		Tag "$AMMO_ID24FUELTANK";
	}
	States
	{
	Spawn:
		FTNK A -1;
		Stop;
	}
}
