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
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
