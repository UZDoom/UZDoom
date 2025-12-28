/*
** doomkeys.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
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

class DoomKey : Key
{
	Default
	{
		Radius 20;
		Height 16;
		+NOTDMATCH
	}
}

// Blue key card ------------------------------------------------------------

class BlueCard : DoomKey
{
	Default
	{
		Inventory.Pickupmessage "$GOTBLUECARD";
		Inventory.Icon "STKEYS0";
		Tag "$TAG_BLUECARD";
	}
	States
	{
	Spawn:
		BKEY A 10;
		BKEY B 10 bright;
		loop;
	}
}

// Yellow key card ----------------------------------------------------------

class YellowCard : DoomKey
{
	Default
	{
		Inventory.Pickupmessage "$GOTYELWCARD";
		Inventory.Icon "STKEYS1";
		Tag "$TAG_YELLOWCARD";
	}
	States
	{
	Spawn:
		YKEY A 10;
		YKEY B 10 bright;
		loop;
	}
}

// Red key card -------------------------------------------------------------

class RedCard : DoomKey
{
	Default
	{
		Inventory.Pickupmessage "$GOTREDCARD";
		Inventory.Icon "STKEYS2";
		Tag "$TAG_REDCARD";
	}
	States
	{
	Spawn:
		RKEY A 10;
		RKEY B 10 bright;
		loop;
	}
}

// Blue skull key -----------------------------------------------------------

class BlueSkull : DoomKey
{
	Default
	{
		Inventory.Pickupmessage "$GOTBLUESKUL";
		Inventory.Icon "STKEYS3";
		Tag "$TAG_BLUESKULL";
	}
	States
	{
	Spawn:
		BSKU A 10;
		BSKU B 10 bright;
		loop;
	}
}

// Yellow skull key ---------------------------------------------------------

class YellowSkull : DoomKey
{
	Default
	{
		Inventory.Pickupmessage "$GOTYELWSKUL";
		Inventory.Icon "STKEYS4";
		Tag "$TAG_YELLOWSKULL";
	}
	States
	{
	Spawn:
		YSKU A 10;
		YSKU B 10 bright;
		loop;
	}
}

// Red skull key ------------------------------------------------------------

class RedSkull : DoomKey
{
	Default
	{
		Inventory.Pickupmessage "$GOTREDSKUL";
		Inventory.Icon "STKEYS5";
		Tag "$TAG_REDSKULL";
	}
	States
	{
	Spawn:
		RSKU A 10;
		RSKU B 10 bright;
		loop;
	}
}

