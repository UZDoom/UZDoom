/*
** teleport.zs
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

class TeleportFog : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOTELEPORT
		+NOGRAVITY
		+ZDOOMTRANS
		RenderStyle "Add";
	}
	States
	{
	Spawn:
		TFOG ABABCDEFGHIJ 6 Bright;
		Stop;
	
	Raven:
		TELE ABCDEFGHGFEDC 6 Bright;
		Stop;

	Strife:
		TFOG ABCDEFEDCB 6 Bright;
		Stop;
	}
	
	override void PostBeginPlay ()
	{
		Super.PostBeginPlay ();
		A_StartSound ("misc/teleport", CHAN_BODY);
		switch (gameinfo.gametype)
		{
		case GAME_Hexen:
		case GAME_Heretic:
			SetStateLabel("Raven");
			break;

		case GAME_Strife:
			SetStateLabel("Strife");
			break;
			
		default:
			break;
		}
	}
	
}



class TeleportDest : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOSECTOR
		+DONTSPLASH
		+NOTONAUTOMAP
	}
}

class TeleportDest2 : TeleportDest
{
	default
	{
		+NOGRAVITY
	}
}

class TeleportDest3 : TeleportDest2
{
	default
	{
		-NOGRAVITY
	}
}

