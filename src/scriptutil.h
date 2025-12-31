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
