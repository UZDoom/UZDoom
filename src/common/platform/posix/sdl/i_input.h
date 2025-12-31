/*
** i_input.h
**
** Handles input from keyboard, mouse, and joystick
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
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

#ifndef I_INPUT_H
#define I_INPUT_H

#include "d_eventbase.h"

extern int WaitingForKey;

static void I_CheckGUICapture ();
static void I_CheckNativeMouse ();

void I_JoyConsumeEvent(int instanceID, event_t * event);

#endif
