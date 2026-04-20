/*
** scriptutil.h
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

#pragma once

#include <stdarg.h>
#include "name.h"


class ScriptUtil
{
	static void BuildParameters(va_list ap);
	static void RunFunction(FName function, unsigned paramstart, VMReturn &returns);

public:	
	enum
	{
		End,
		Int,
		Pointer,
		Float,
		String,
		Class,
	};

	static int Exec(FName functionname, ...);
	static void Clear();
};
