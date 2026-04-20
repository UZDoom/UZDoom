/*
** chexkeys.zs
**
** These are merely renames of the Doom cards
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

class ChexBlueCard : BlueCard
{
	Default
	{
		inventory.pickupmessage "$GOTCBLUEKEY";
	}
}

class ChexYellowCard : YellowCard
{
	Default
	{
		inventory.pickupmessage "$GOTCYELLOWKEY";
	}
}

class ChexRedCard : RedCard
{
	Default
	{
		inventory.pickupmessage "$GOTCREDKEY";
	}
}
