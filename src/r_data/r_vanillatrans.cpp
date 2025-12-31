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
#ifdef _DEBUG
#include "c_dispatch.h"
#endif

bool r_UseVanillaTransparency;
CVAR(Int, r_vanillatrans, 0, CVAR_ARCHIVE)
TMap<FName, bool> AutoTrans = {};

namespace
{
	bool firstTime = true;
	bool foundDehacked = false;
	bool foundDecorate = false;
	bool foundZScript = false;
}
#ifdef _DEBUG
CCMD (debug_checklumps)
{
	Printf("firstTime: %d\n", firstTime);
	Printf("foundDehacked: %d\n", foundDehacked);
	Printf("foundDecorate: %d\n", foundDecorate);
	Printf("foundZScript: %d\n", foundZScript);
}
#endif

void UpdateVanillaTransparency()
{
	firstTime = true;
}

bool UseVanillaTransparency()
{
	if (firstTime)
	{
		int lastlump = 0;
		fileSystem.FindLump("ZSCRIPT", &lastlump); // ignore first ZScript
		if (fileSystem.FindLump("ZSCRIPT", &lastlump) == -1) // no loaded ZScript
		{
			lastlump = 0;
			foundDehacked = fileSystem.FindLump("DEHACKED", &lastlump) != -1;
			lastlump = 0;
			foundDecorate = fileSystem.FindLump("DECORATE", &lastlump) != -1;
			foundZScript = false;
		}
		else
		{
			foundZScript = true;
			foundDehacked = false;
			foundDecorate = false;
		}
		firstTime = false;
	}

	switch (r_vanillatrans)
	{
		case 0: return false;
		case 1: return true;
		default:
		if (foundDehacked)
			return true;
		if (foundDecorate)
			return false;
		return r_vanillatrans == 3;
	}
}
