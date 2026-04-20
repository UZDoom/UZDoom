/*
** p_saveg.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2016 Christoph Oelckers
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

#ifndef __P_SAVEG_H__
#define __P_SAVEG_H__

class FSerializer;

// Persistent storage/archiving.
// These are the load / save game routines.
// Also see farchive.(h|cpp)
void P_DestroyThinkers(bool hubLoad);

void P_ReadACSDefereds (FSerializer &);
void P_WriteACSDefereds (FSerializer &);

#endif // __P_SAVEG_H__
