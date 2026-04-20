/*
** messagebox.cpp
**
** Confirmation, notification screens
**
**---------------------------------------------------------------------------
**
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

#include <ctype.h>
#include "menu.h"
#include "gstrings.h"
#include "i_video.h"
#include "c_dispatch.h"
#include "vm.h"
#include "menustate.h"

FName MessageBoxClass = NAME_MessageBoxMenu;

CVAR(Bool, m_quickexit, false, CVAR_ARCHIVE)

typedef void(*hfunc)();
DEFINE_ACTION_FUNCTION(DMessageBoxMenu, CallHandler)
{
	PARAM_SELF_PROLOGUE(DMenu);
	PARAM_POINTER(unused, void);
	auto handler = reinterpret_cast<hfunc>(self->PointerVar<void>(NAME_InternalHandler));
	if (handler != nullptr)
		handler();
	return 0;
}

//=============================================================================
//
//
//
//=============================================================================

DMenu *CreateMessageBoxMenu(DMenu *parent, const char *message, int messagemode, bool playsound, FName action = NAME_None, hfunc handler = nullptr)
{
	auto c = PClass::FindClass(MessageBoxClass);
	if (!c->IsDescendantOf(NAME_MessageBoxMenu)) c = PClass::FindClass(NAME_MessageBoxMenu);
	auto p = c->CreateNew();
	FString namestr = message;

	IFVIRTUALPTRNAME(p, NAME_MessageBoxMenu, Init)
	{
		p->PointerVar<void>(NAME_InternalHandler) = reinterpret_cast<void*>(handler);
		VMValue params[] = { p, parent, &namestr, messagemode, playsound, action.GetIndex(), reinterpret_cast<void*>(handler) };
		VMCall(func, params, countof(params), nullptr, 0);
		return (DMenu*)p;
	}
	return nullptr;
}

//=============================================================================
//
//
//
//=============================================================================

void M_StartMessage(const char *message, int messagemode, FName action)
{
	if (CurrentMenu == NULL) 
	{
		// only play a sound if no menu was active before
		M_StartControlPanel(menuactive == MENU_Off);
	}
	DMenu *newmenu = CreateMessageBoxMenu(CurrentMenu, message, messagemode, false, action);
	newmenu->mParentMenu = CurrentMenu;
	M_ActivateMenu(newmenu);
}

DEFINE_ACTION_FUNCTION(DMenu, StartMessage)
{
	PARAM_PROLOGUE;
	PARAM_STRING(msg);
	PARAM_INT(mode);
	PARAM_NAME(action);
	M_StartMessage(msg.GetChars(), mode, action);
	return 0;
}
