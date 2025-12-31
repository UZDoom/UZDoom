/*
** m_haptics.h
**
** Haptic feedback implementation
**
**---------------------------------------------------------------------------
**
** Copyright 2025 Marcus Minhorst
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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
*/

#ifndef M_HAPTICS_H
#define M_HAPTICS_H

#include "name.h"

enum EHapticCompat {
	HAPTCOMPAT_WARN,
	HAPTCOMPAT_NONE,
	HAPTCOMPAT_MATCH,
	HAPTCOMPAT_ALL,

	NUM_HAPTCOMPAT
};

struct Haptics {
	int ticks;
	double high_frequency;
	double low_frequency;
	double left_trigger;
	double right_trigger;
};

void I_Rumble(double high_freq, double low_freq, double left_trig, double right_trig);

void Joy_AddRumbleType(const FName idenifier, const struct Haptics data);
void Joy_AddRumbleAlias(const FName alias, const FName actual);
void Joy_MapRumbleType(const FName sound, const FName idenifier);
void Joy_ResetRumbleMapping();
void Joy_ReadyRumbleMapping();
void Joy_RumbleTick();
void Joy_Rumble(
	const FName source, int tic_count,
	double high_frequency, double low_frequency,
	double left_trigger = 0, double right_trigger = 0,
	double attenuation = 0
);
void Joy_Rumble(const FName identifier, double attenuation = 0);

#endif
