/*
** v_palette.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __V_PALETTE_H__
#define __V_PALETTE_H__

#include "doomtype.h"
#include "c_cvars.h"
#include "palutil.h"
#include "vectors.h"



// The color overlay to use for depleted items
#define DIM_OVERLAY MAKEARGB(170,0,0,0)

int ReadPalette(int lumpnum, uint8_t *buffer);
void InitPalette ();

EXTERN_CVAR (Int, paletteflash)
enum PaletteFlashFlags
{
	PF_HEXENWEAPONS		= 1,
	PF_POISON			= 2,
	PF_ICE				= 4,
	PF_HAZARD			= 8,
};

class player_t;
struct sector_t;

void V_AddBlend (float r, float g, float b, float a, float v_blend[4]);
void V_AddPlayerBlend (player_t *CPlayer, float blend[4], float maxinvalpha, int maxpainblend);
// Dim part of the canvas
FVector4 V_CalcBlend(sector_t* viewsector, PalEntry* modulateColor);
void V_DrawBlend(sector_t* viewsector);

#endif //__V_PALETTE_H__
