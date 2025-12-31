/*
** chexitems.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1996-1997 Digital Café
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


// General Pickups ============================================================

// Health ---------------------------------------------------------------------

class GlassOfWater : HealthBonus
{
	Default
	{
		inventory.pickupmessage "$GOTWATER";
		tag "$TAG_GLASSOFWATER";
	}
}

class BowlOfFruit : Stimpack
{
	Default
	{
		inventory.pickupmessage "$GOTFRUIT";
		tag "$TAG_BOWLOFFRUIT";
	}
}

class BowlOfVegetables : Medikit
{
	Default
	{
		inventory.pickupmessage "$GOTVEGETABLES";
		health.lowmessage 25, "$GOTVEGETABLESNEED";
		tag "$TAG_BOWLOFVEGETABLES";
	}
}

class SuperchargeBreakfast : Soulsphere
{
	Default
	{
		inventory.pickupmessage "$GOTBREAKFAST";
		tag "$TAG_BREAKFAST";
	}
}

// Armor ----------------------------------------------------------------------

class SlimeRepellent : ArmorBonus
{
	Default
	{
		inventory.pickupmessage "$GOTREPELLENT";
		tag "$TAG_REPELLENT";
	}
}

class ChexArmor : GreenArmor
{
	Default
	{
		inventory.pickupmessage "$GOTCHEXARMOR";
		tag "$TAG_CHEXARMOR";
	}
}

class SuperChexArmor : BlueArmor
{
	Default
	{
		inventory.pickupmessage "$GOTSUPERCHEXARMOR";
		tag "$TAG_SUPERCHEXARMOR";
	}
}

// Powerups ===================================================================

class ComputerAreaMap : Allmap
{
	Default
	{
		inventory.pickupmessage "$GOTCHEXMAP";
		tag "$TAG_CHEXMAP";
	}
}

class SlimeProofSuit : RadSuit
{
	Default
	{
		inventory.pickupmessage "$GOTSLIMESUIT";
		tag "$TAG_SLIMESUIT";
	}
}

class Zorchpack : Backpack
{
	Default
	{
		inventory.pickupmessage "$GOTZORCHPACK";
		tag "$TAG_ZORCHPACK";
	}
}
