/*
** chexplayer.zs
**
** Chex Warrior
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

class ChexPlayer : DoomPlayer
{
	Default
	{
		player.displayname "Chex Warrior";
		player.crouchsprite "";
		player.colorrange 192, 207; //Not perfect, but its better than everyone being blue.
		player.startitem "MiniZorcher";
		player.startitem "Bootspoon";
		player.startitem "MiniZorchRecharge", 50;
		player.damagescreencolor "60 b0 58";
		player.WeaponSlot 1, "Bootspoon", "SuperBootspork";
		player.WeaponSlot 2, "MiniZorcher";
		player.WeaponSlot 3, "LargeZorcher", "SuperLargeZorcher";
		player.WeaponSlot 4, "RapidZorcher";
		player.WeaponSlot 5, "ZorchPropulsor";
		player.WeaponSlot 6, "PhasingZorcher";
		player.WeaponSlot 7, "LAZDevice";
		
		Player.Colorset 0, "$TXT_COLOR_LIGHTBLUE",	0xC0, 0xCF,  0xC2;
		Player.Colorset 1, "$TXT_COLOR_GREEN",		0x70, 0x7F,  0x72;
		Player.Colorset 2, "$TXT_COLOR_GRAY",		0x60, 0x6F,  0x62;
		Player.Colorset 3, "$TXT_COLOR_BROWN",		0x40, 0x4F,  0x42;
		Player.Colorset 4, "$TXT_COLOR_RED",		0x20, 0x2F,  0x22;
		Player.Colorset 5, "$TXT_COLOR_LIGHTGRAY",	0x58, 0x67,  0x5A;
		Player.Colorset 6, "$TXT_COLOR_LIGHTBROWN",	0x38, 0x47,  0x3A;
		Player.Colorset 7, "$TXT_COLOR_LIGHTRED",	0xB0, 0xBF,  0xB2;
	}
	
}
