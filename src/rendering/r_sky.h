/*
** r_sky.h
**
** Sky rendering.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
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

#ifndef __R_SKY_H__
#define __R_SKY_H__

#include <utility>
#include <stdint.h>
#include "textureid.h"

struct FLevelLocals;

extern FTextureID	skyflatnum;
extern int		freelookviewheight;

#define SKYSTRETCH_HEIGHT 228

// Called whenever the sky changes.
void InitSkyMap(FLevelLocals *Level);
void R_InitSkyMap();
void R_UpdateSky (double ticFrac);


#endif //__R_SKY_H__
