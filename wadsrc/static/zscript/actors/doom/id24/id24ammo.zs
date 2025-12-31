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
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
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
