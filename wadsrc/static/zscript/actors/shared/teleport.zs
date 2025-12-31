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
** SPDX-License-Identifier: GPL-3.0-or-later
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

