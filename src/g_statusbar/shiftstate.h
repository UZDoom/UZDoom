/*
** shiftstate.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2022 Christoph Oelckers
** Copyright 2022-2025 GZDoom Maintainers and Contributors
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
#include "d_event.h"

class ShiftState
{
	bool ShiftStatus = false;

public:

	bool ShiftPressed()
	{
		return ShiftStatus;
	}

	void AddEvent(const event_t *ev)
	{
		if ((ev->type == EV_KeyDown || ev->type == EV_KeyUp) && (ev->data1 == KEY_LSHIFT || ev->data1 == KEY_RSHIFT))
		{
			ShiftStatus = ev->type == EV_KeyDown;
		}
	}

	void Clear()
	{
		ShiftStatus = false;
	}


};

inline ShiftState shiftState;
