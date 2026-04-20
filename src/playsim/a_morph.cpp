/*
** a_morph.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018 Christoph Oelckers
** Copyright 2018-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "info.h"
#include "actor.h"
#include "vm.h"

bool P_MorphActor(AActor *activator, AActor *victim, PClassActor *ptype, PClassActor *mtype, int duration, int style, PClassActor *enter_flash, PClassActor *exit_flash)
{
	IFVIRTUALPTR(victim, AActor, Morph)
	{
		VMValue params[] = { victim, activator, ptype, mtype, duration, style, enter_flash, exit_flash };
		int retval;
		VMReturn ret(&retval);
		VMCall(func, params, countof(params), &ret, 1);
		return !!retval;
	}
	return false;
}

bool P_UnmorphActor(AActor *activator, AActor *morphed, int flags, bool force)
{
	IFVIRTUALPTR(morphed, AActor, Unmorph)
	{
		VMValue params[] = { morphed, activator, flags, force };
		int retval;
		VMReturn ret(&retval);
		VMCall(func, params, countof(params), &ret, 1);
		return !!retval;
	}
	return false;
}


