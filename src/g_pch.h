/*
** g_pch.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <miniz.h>
#include <new>
#include <algorithm>
#include <forward_list>
#include <sys/stat.h>
#include <sys/types.h>
#include <cassert>
#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#endif // _MSC_VER
#include <limits>
#include <memory>
#include <tuple>
#include <vector>
#include <utility>
#include <functional>

// These two headers get included nearly everywhere so it doesn't matter if changing them forces a few more recompiles.
// The overall savings from PCHing them are more significant.
#include "tarray.h"
#include "zstring.h"
