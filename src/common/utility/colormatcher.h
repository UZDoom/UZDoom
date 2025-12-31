/*
** colormatcher.h
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
** Once upon a time, this tried to be a fast closest color finding system.
** It was, but the results were not as good as I would like, so I didn't
** actually use it. But I did keep the code around in case I ever felt like
** revisiting the problem. I never did, so now it's relegated to the mists
** of SVN history, and this is just a thin wrapper around BestColor().
**
*/

#ifndef __COLORMATCHER_H__
#define __COLORMATCHER_H__

#include "palutil.h"

int BestColor (const uint32_t *pal_in, int r, int g, int b, int first, int num, const uint8_t* indexmap);

class FColorMatcher
{
public:

	void SetPalette(PalEntry* palette) { Pal = palette; }
	void SetPalette (const uint32_t *palette) { Pal = reinterpret_cast<const PalEntry*>(palette); }
	void SetIndexMap(const uint8_t* index) { indexmap = index; startindex = index ? 0 : 1; }
	uint8_t Pick (int r, int g, int b)
	{
		if (Pal == nullptr)
			return 1;

		return (uint8_t)BestColor ((uint32_t *)Pal, r, g, b, startindex, 255, indexmap);
	}

	uint8_t Pick (PalEntry pe)
	{
		return Pick(pe.r, pe.g, pe.b);
	}

private:
	const PalEntry *Pal = nullptr;
	const uint8_t* indexmap = nullptr;
	int startindex = 1;
};

extern FColorMatcher ColorMatcher;


#endif //__COLORMATCHER_H__
