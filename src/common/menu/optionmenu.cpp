/*
** optionmenu.cpp
**
** Handler class for the option menus and associated items
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2017 Christoph Oelckers
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

#include "c_cvars.h"
#include "menu.h"
#include "vm.h"

// by request of Nash
CVARD(Bool, silence_menu_scroll, true, CVAR_GLOBALCONFIG | CVAR_ARCHIVE, "Silences cursor movement when using mouse wheel");
CVARD(Bool, silence_menu_hover, true, CVAR_GLOBALCONFIG | CVAR_ARCHIVE, "Silences cursor movement when implicitly selecting with mouse");

CVARD(Int, menu_overscroll, 8, CVAR_GLOBALCONFIG | CVAR_ARCHIVE, "Number of lines you can scroll past the bottom of a menu");

//=============================================================================
//
//
//
//=============================================================================

DMenuItemBase *DOptionMenuDescriptor::GetItem(FName name)
{
	for(unsigned i=0;i<mItems.Size(); i++)
	{
		FName nm = mItems[i]->mAction;
		if (nm == name) return mItems[i];
	}
	return NULL;
}

void SetCVarDescription(FBaseCVar* cvar, const FString* label)
{
	cvar->AddDescription(*label);
}

DEFINE_ACTION_FUNCTION_NATIVE(_OptionMenuItemOption, SetCVarDescription, SetCVarDescription)
{
	PARAM_PROLOGUE;
	PARAM_POINTER(cv, FBaseCVar);
	PARAM_STRING(label);
	SetCVarDescription(cv, &label);
	return 0;
}
