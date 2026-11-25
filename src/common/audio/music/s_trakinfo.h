/*
** s_trakinfo.h
**
** TRAKINFO lump parser and soundtrack lookup
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <cstddef>

enum ETrakInfoMode : int
{
	TRAKINFO_Original = 0,
	TRAKINFO_MIDI = 1,
	TRAKINFO_Remixed = 2,
};

void S_ParseTrakInfo();
void S_ClearTrakInfo();
bool S_HasTrakInfo();

// returns the replacement lump name for the requested soundtrack mode, or nullptr if none
const char *S_TrakInfoFindReplacement(const void *data, size_t len, int mode);

// returns the volume multiplier specified in TRAKINFO, or -1.f if there is none
float S_TrakInfoFindVolume(const void *data, size_t len);
