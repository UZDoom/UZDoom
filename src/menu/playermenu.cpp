/*
** playermenu.cpp
**
** The player setup menu's setters. These are native for security purposes.
**
**---------------------------------------------------------------------------
**
** Copyright 2001-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include "menu.h"
#include "gi.h"
#include "c_dispatch.h"
#include "teaminfo.h"
#include "r_state.h"
#include "vm.h"
#include "d_player.h"

EXTERN_CVAR(Int, team)
EXTERN_CVAR(Float, autoaim)
EXTERN_CVAR(Bool, neverswitchonpickup)
EXTERN_CVAR(Bool, cl_run)

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, ColorChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(r);
	PARAM_INT(g);
	PARAM_INT(b);
	// only allow if the menu is active to prevent abuse.
	if (DMenu::InMenu)
	{
		char command[24];
		players[consoleplayer].userinfo.ColorChanged(MAKERGB(r, g, b));
		mysnprintf(command, countof(command), "color \"%02x %02x %02x\"", r, g, b);
		C_DoCommand(command);
	}
	return 0;
}


//=============================================================================
//
// access to the player config is done natively, so that broader access
// functions do not need to be exported.
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, PlayerNameChanged)
{
	PARAM_PROLOGUE;
	PARAM_STRING(s);
	const char *pp = s.GetChars();
	FString command("name \"");

	if (DMenu::InMenu)
	{
		// Escape any backslashes or quotation marks before sending the name to the console.
		for (auto p = pp; *p != '\0'; ++p)
		{
			if (*p == '"' || *p == '\\')
			{
				command << '\\';
			}
			command << *p;
		}
		command << '"';
		C_DoCommand(command.GetChars());
	}
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, ColorSetChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(sel);
	if (DMenu::InMenu)
	{
		players[consoleplayer].userinfo.ColorSetChanged(sel);
		char command[24];
		mysnprintf(command, countof(command), "colorset %d", sel);
		C_DoCommand(command);
	}
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, ClassChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(sel);
	PARAM_POINTER(cls, FPlayerClass);
	if (DMenu::InMenu)
	{
		const char *pclass = sel == -1 ? "Random" : GetPrintableDisplayName(cls->Type).GetChars();
		players[consoleplayer].userinfo.PlayerClassChanged(pclass);
		cvar_set("playerclass", pclass);
	}
	return 0;
}


//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, SkinChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(sel);
	if (DMenu::InMenu)
	{
		players[consoleplayer].userinfo.SkinNumChanged(sel);
		cvar_set("skin", Skins[sel].Name.GetChars());
	}
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, AutoaimChanged)
{
	PARAM_PROLOGUE;
	PARAM_FLOAT(val);
	// only allow if the menu is active to prevent abuse.
	if (DMenu::InMenu)
	{
		autoaim = float(val);
	}
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, TeamChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(val);
	// only allow if the menu is active to prevent abuse.
	if (DMenu::InMenu)
	{
		team = val == 0 ? TEAM_NONE : val - 1;
	}
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, GenderChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(v);
	// only allow if the menu is active to prevent abuse.
	if (DMenu::InMenu)
	{
		switch(v)
		{
		case 0: cvar_set("gender", "male");    break;
		case 1: cvar_set("gender", "female");  break;
		case 2: cvar_set("gender", "neutral"); break;
		case 3: cvar_set("gender", "other");   break;
		}
	}
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, SwitchOnPickupChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(v);
	// only allow if the menu is active to prevent abuse.
	if (DMenu::InMenu)
	{
		neverswitchonpickup = !!v;
	}
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DPlayerMenu, AlwaysRunChanged)
{
	PARAM_PROLOGUE;
	PARAM_INT(v);
	// only allow if the menu is active to prevent abuse.
	if (DMenu::InMenu)
	{
		cl_run = !!v;
	}
	return 0;
}
