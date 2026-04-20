/*
** haptics.zs
**
** Interface to buzz the gamepad a bit
**
**---------------------------------------------------------------------------
**
** Copyright 2025 Marcus Minhorst
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class Haptics
{
    native static void RumbleDirect(int tic_count, float high_frequency, float low_frequency, float left_trigger, float right_trigger);
    native static void Rumble(Name identifier);
}
