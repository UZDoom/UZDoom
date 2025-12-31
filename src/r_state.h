/*
** r_state.h
**
** Refresh/render internal state variables (global).
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef __R_STATE_H__
#define __R_STATE_H__

// Need data structure definitions.
#include "doomtype.h"
#include "r_defs.h"
#include "r_data/sprites.h"

//
// Refresh internal data structures,
//	for rendering.
//

extern int				viewwindowx;
extern int				viewwindowy;
extern int				viewwidth;
extern int				viewheight;

//
// Lookup tables for map data.
//
extern TArray<spritedef_t> sprites;
extern uint32_t NumStdSprites;

//
// POV data.
//

int R_FindSkin (const char *name, int pclass);	// [RH] Find a skin

#endif // __R_STATE_H__
