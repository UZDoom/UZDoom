/*
** p_tick.h
**
** Ticker.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef __P_TICK__
#define __P_TICK__

// Called by C_Ticker,
// can call G_PlayerExited.
// Carries out all thinking of monsters and players.
void P_Ticker (void);
bool P_CheckTickerPaused ();


#endif
