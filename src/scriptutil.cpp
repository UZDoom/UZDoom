/*
** scriptutil.cpp
**
** generalized interface for implementing ACS/FS functions in ZScript.
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

#include "tarray.h"
#include "dobject.h"
#include "vm.h"
#include "scriptutil.h"
#include "p_acs.h"
#include "actorinlines.h"


static TArray<VMValue> parameters;
static TMap<FName, VMFunction*> functions;

void ScriptUtil::Clear()
{
	parameters.Clear();
	functions.Clear();
}

void ScriptUtil::BuildParameters(va_list ap)
{
	for(int type = va_arg(ap, int); type != End; type = va_arg(ap, int))
	{
		switch (type)
		{
			case Int:
				parameters.Push(VMValue(va_arg(ap, int)));
				break;
				
			case Pointer:
			case Class:		// this is just a pointer.
			case String:	// must be passed by reference to a persistent location!
				parameters.Push(VMValue(va_arg(ap, void*)));
				break;
				
			case Float:
				parameters.Push(VMValue(va_arg(ap, double)));
				break;
		}
	}
}

void ScriptUtil::RunFunction(FName functionname, unsigned paramstart, VMReturn &returns)
{
	VMFunction *func = nullptr;
	auto check = functions.CheckKey(functionname);
	if (!check)
	{
		func = PClass::FindFunction(NAME_ScriptUtil, functionname);
		if (func == nullptr) 
		{
			I_Error("Call to undefined function ScriptUtil.%s", functionname.GetChars());
		}
		functions.Insert(functionname, func);
	}
	else func = *check;

	VMCall(func, &parameters[paramstart], parameters.Size() - paramstart, &returns, 1);
}

int ScriptUtil::Exec(FName functionname, ...)
{
	unsigned paramstart = parameters.Size();
	va_list ap;
	va_start(ap, functionname);
	try
	{
		BuildParameters(ap);
		int ret = 0;
		VMReturn returns(&ret);
		RunFunction(functionname, paramstart, returns);
		va_end(ap);
		parameters.Clamp(paramstart);
		return ret;
	}
	catch(...)
	{
		va_end(ap);
		parameters.Clamp(paramstart);
		throw;
	}
}
