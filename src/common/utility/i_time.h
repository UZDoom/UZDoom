/*
** i_time.h
**
** Implements the timer
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2017 Magnus Norddahl
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

#include <stdint.h>

extern int GameTicRate;
extern double TimeScale;

void I_InitTime();

// Called by D_DoomLoop, sets the time for the current frame
void I_SetFrameTime();

// Called by D_DoomLoop, returns current time in tics.
int I_GetTime(double const ticrate = GameTicRate);
// same, but using nanoseconds
uint64_t I_GetTimeNS();

double I_GetTimeFrac(double const ticrate = GameTicRate);

// like I_GetTime, except it waits for a new tic before returning
int I_WaitForTic(int prevtic, double const ticrate = GameTicRate);

// Freezes tic counting temporarily. While frozen, calls to I_GetTime()
// will always return the same value.
// You must also not call I_WaitForTic() while freezing time, since the
// tic will never arrive (unless it's the current one).
void I_FreezeTime(bool frozen);

// [RH] Returns millisecond-accurate time
uint64_t I_msTime();

// [RH] Returns nanosecond-accurate time in milliseconds
double I_msTimeF(void);

// [SP] Returns millisecond-accurate time from start
uint64_t I_msTimeFS();

// Nanosecond-accurate time
uint64_t I_nsTime();

// Reset the timer after a lengthy operation
void I_ResetFrameTime();

// Return a decimal fraction to scale input operations at framerate
double I_GetInputFrac();

// Reset the last input check to after a lengthy operation
void I_ResetInputTime();

// Pause a bit.
// [RH] Despite the name, it apparently never waited for the VBL, even in
// the original DOS version (if the Heretic/Hexen source is any indicator).
void I_WaitVBL(int count);

double GetDeltaTime(bool current = false);
double GetPhysicsTimeStep();
void ClearPrevTime();
void SetDeltaTime();
