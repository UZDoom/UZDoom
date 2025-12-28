/*
** doomtype.h
**
** Simple basic typedefs, isolated here to make it easier separating modules.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2008-2016 Christoph Oelckers
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

#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#ifdef _MSC_VER
// VC++ does not define PATH_MAX, but the Windows headers do define MAX_PATH.
// However, we want to avoid including the Windows headers in most of the
// source files, so we can't use it. So define PATH_MAX to be what MAX_PATH
// currently is:
#define PATH_MAX 260

// Disable warning about using unsized arrays in structs. It supports it just
// fine, and so do Clang and GCC, but the latter two don't warn about it.
#pragma warning(disable:4200)
#endif

#include <limits.h>
#include <tuple>
#include <algorithm>
#include "tarray.h"
#include "name.h"
#include "zstring.h"
#include "cmdlib.h"

class PClassActor;
typedef TMap<int, PClassActor *> FClassMap;

#include "basics.h"
#include "printf.h"

// Bounding box coordinate storage.
#include "palentry.h"
#include "textureid.h"

enum class ELightMode : int8_t
{
	NotSet = -1,
	LinearStandard = 0,
	DoomBright = 1,
	Doom = 2,
	DoomDark = 3,
	DoomLegacy = 4,
	Build = 5,
	ZDoomSoftware = 8,
	DoomSoftware = 16
};

#endif
