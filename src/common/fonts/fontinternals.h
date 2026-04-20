/*
** fontinternals.h
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

#pragma once

#include <stdint.h>
#include "tarray.h"

struct TranslationParm
{
	short RangeStart;	// First level for this range
	short RangeEnd;		// Last level for this range
	uint8_t Start[3];		// Start color for this range
	uint8_t End[3];		// End color for this range
};

struct TempParmInfo
{
	unsigned int StartParm[2];
	unsigned int ParmLen[2];
	int Index;
};
struct TempColorInfo
{
	FName Name;
	unsigned int ParmInfo;
	PalEntry LogColor;
};

struct TranslationMap
{
	FName Name;
	int Number;
};

extern TArray<TranslationParm> TranslationParms[2];
extern TArray<TranslationMap> TranslationLookup;
extern TArray<PalEntry> TranslationColors;

class FImageSource;
