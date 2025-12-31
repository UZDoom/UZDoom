/*
** resolutionmenu.cpp
**
** Basic Custom Resolution Selector for the Menu
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2018 Rachael Alexanderson
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

#include "c_dispatch.h"
#include "c_cvars.h"
#include "v_video.h"
#include "menu.h"
#include "printf.h"

CVAR(Int, menu_resolution_custom_width, 640, 0)
CVAR(Int, menu_resolution_custom_height, 480, 0)

EXTERN_CVAR(Bool, vid_fullscreen)
EXTERN_CVAR(Bool, win_maximized)
EXTERN_CVAR(Float, vid_scale_custompixelaspect)
EXTERN_CVAR(Int, vid_scale_customwidth)
EXTERN_CVAR(Int, vid_scale_customheight)
EXTERN_CVAR(Int, vid_scalemode)
EXTERN_CVAR(Float, vid_scalefactor)

CCMD (menu_resolution_set_custom)
{
	if (argv.argc() > 2)
	{
		menu_resolution_custom_width = atoi(argv[1]);
		menu_resolution_custom_height = atoi(argv[2]);
	}
	else
	{
		Printf("This command is not meant to be used outside the menu! But if you want to use it, please specify <x> and <y>.\n");
	}
	M_PreviousMenu();
}

CCMD (menu_resolution_commit_changes)
{
	int do_fullscreen = vid_fullscreen;
	if (argv.argc() > 1)
	{
		do_fullscreen = atoi(argv[1]);
	}

	if (do_fullscreen == false)
	{
		vid_scalemode = 0;
		vid_scalefactor = 1.;
		screen->SetWindowSize(menu_resolution_custom_width, menu_resolution_custom_height);
		V_OutputResized(screen->GetClientWidth(), screen->GetClientHeight());
	}
	else
	{
		vid_fullscreen = true;
		vid_scalemode = 5;
		vid_scalefactor = 1.;
		vid_scale_customwidth = *menu_resolution_custom_width;
		vid_scale_customheight = *menu_resolution_custom_height;
		vid_scale_custompixelaspect = 1.0;
	}
}


