/*
** hu_stuff.h
**
** Head up display
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
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

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "doomtype.h"

struct event_t;
class player_t;

//
// Chat routines
//

void CT_Init (void);
bool CT_Responder (event_t* ev);
void CT_Drawer (void);

// [RH] Draw deathmatch scores

void HU_DrawScores(double ticFrac);

extern bool bScoreboardToggled;

#endif
