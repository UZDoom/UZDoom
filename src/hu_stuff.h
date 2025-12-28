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
