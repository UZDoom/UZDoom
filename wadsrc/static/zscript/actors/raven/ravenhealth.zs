/*
** ravenhealth.zs
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
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class CrystalVial : Health
{
	Default
	{
		+FLOATBOB
		Inventory.Amount 10;
		Inventory.PickupMessage "$TXT_ITEMHEALTH";
		Tag "$TAG_ITEMHEALTH";
	}
	States
	{
	Spawn:
		PTN1 ABC 3;
		Loop;
	}
}

