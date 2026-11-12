/*
** r_vanillatrans.cpp
**
** Figures out whether to turn off transparency for certain native game objects
**
**---------------------------------------------------------------------------
**
** Copyright 2017 Rachael Alexanderson
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

#include "c_cvars.h"
#include "filesystem.h"
#include "doomtype.h"

int r_UseVanillaTransparency = 0;
CVAR(Int, r_vanillatrans, 0, CVAR_ARCHIVE)
TMap<FName, bool> AutoTrans = {};
bool bDehackedTransPresent = false;
bool bModdedTransPresent = false;

int UseVanillaTransparency()
{
	switch (r_vanillatrans)
	{
		case 2:
			return bDehackedTransPresent;
		case 3:
			return !bModdedTransPresent;
		default:
			return clamp<int>(r_vanillatrans, -1, 1);
	}
}
