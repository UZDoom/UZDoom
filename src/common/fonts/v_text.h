/*
** v_text.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#include "v_font.h"

struct FBrokenLines
{
	unsigned Width;
	FString  Text;
};

TArray<FBrokenLines> V_BreakLines (FFont *font, int maxwidth, const uint8_t *str, bool preservecolor = false);
inline TArray<FBrokenLines> V_BreakLines (FFont *font, int maxwidth, const char *str, bool preservecolor = false)
 { return V_BreakLines (font, maxwidth, (const uint8_t *)str, preservecolor); }
inline TArray<FBrokenLines> V_BreakLines (FFont *font, int maxwidth, const FString &str, bool preservecolor = false)
 { return V_BreakLines (font, maxwidth, (const uint8_t *)str.GetChars(), preservecolor); }
