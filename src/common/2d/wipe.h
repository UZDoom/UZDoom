/*
** wipe.h
**
** Screen wipe
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2022 Christoph Oelckers
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

#ifndef __F_WIPE_H__
#define __F_WIPE_H__

#include "stdint.h"
#include <functional>

class FTexture;

enum
{
	wipe_None,			// don't bother
	wipe_Melt,			// weird screen melt
	wipe_Burn,			// fade in shape of fire
	wipe_Fade,			// crossfade from old to new
	wipe_NUMWIPES
};

void PerformWipe(FTexture* startimg, FTexture* endimg, int wipe_type, bool stopsound, std::function<void()> overlaydrawer);


#endif
