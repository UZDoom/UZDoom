/*
** d_dehacked.h
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

#ifndef __D_DEHACK_H__
#define __D_DEHACK_H__

enum DehLumpSource
{
	FromIWAD,
	FromPWADs
};

enum DehFlags
{
	DEH_SKIP_BEX_STRINGS_IF_LANGUAGE = 1,
};

int D_LoadDehLumps(DehLumpSource source, int flags);
bool D_LoadDehLump(int lumpnum, int flags);
bool D_LoadDehFile(const char *filename, int flags);
void FinishDehPatch ();

#endif //__D_DEHACK_H__
