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
** SPDX-License-Identifier: GPL-3.0-or-later
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
