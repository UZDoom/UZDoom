/*
** hereticarmor.zs
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

// Silver Shield (Shield1) --------------------------------------------------

Class SilverShield : BasicArmorPickup
{
	Default
	{
		+FLOATBOB
		Inventory.Pickupmessage "$TXT_ITEMSHIELD1";
		Tag "$TAG_ITEMSHIELD1";
		Inventory.Icon "SHLDA0";
		Armor.Savepercent 50;
		Armor.Saveamount 100;
	}
	States
	{
	Spawn:
		SHLD A -1;
		stop;
	}
}

// Enchanted shield (Shield2) -----------------------------------------------

Class EnchantedShield : BasicArmorPickup
{
	Default
	{
		+FLOATBOB
		Inventory.Pickupmessage "$TXT_ITEMSHIELD2";
		Tag "$TAG_ITEMSHIELD2";
		Inventory.Icon "SHD2A0";
		Armor.Savepercent 75;
		Armor.Saveamount 200;
	}
	States
	{
	Spawn:
		SHD2 A -1;
		stop;
	}
}

