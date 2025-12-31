/* 
** r_vanillatrans.h
**
**
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

#pragma once

void UpdateVanillaTransparency();
bool UseVanillaTransparency();
extern bool r_UseVanillaTransparency;
extern TMap<FName, bool> AutoTrans;
EXTERN_CVAR(Int, r_vanillatrans)
