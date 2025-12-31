/*
** colormaps.h
**
** common Colormap handling
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

#ifndef __RES_CMAP_H
#define __RES_CMAP_H

#include "doomtype.h"

struct lightlist_t;

void R_InitColormaps (bool allowCustomColormap = false);
void R_DeinitColormaps ();

void R_UpdateInvulnerabilityColormap ();

uint32_t R_ColormapNumForName(const char *name);	// killough 4/4/98
void R_SetDefaultColormap (const char *name);	// [RH] change normal fadetable
uint32_t R_BlendForColormap (uint32_t map);			// [RH] return calculated blend for a colormap

struct FakeCmap 
{
	char name[8];
	PalEntry blend;
	int lump;
};

extern TArray<FakeCmap> fakecmaps;

#include "fcolormap.h"

// For hardware-accelerated weapon sprites in colored sectors
struct FColormapStyle
{
	PalEntry Color;
	PalEntry Fade;
	int Desaturate;
	float FadeLevel;
};

// some utility functions to store special colormaps in powerup blends
#define SPECIALCOLORMAP_MASK 0x00b60000

inline uint32_t MakeSpecialColormap(int index)
{
	assert(index >= 0 && index < 65536);
	return index | SPECIALCOLORMAP_MASK;
}

#endif
