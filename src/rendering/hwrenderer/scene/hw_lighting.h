/*
** hw_lighting.h
**
** Light level / fog management / dynamic lights
**
**---------------------------------------------------------------------------
**
** Copyright 2002-2018 Christoph Oelckers
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

#include "c_cvars.h"
#include "v_palette.h"

#include "r_utility.h"

struct Colormap;

EXTERN_CVAR(Int, r_extralight)

inline int hw_ClampLight(int lightlevel)
{
	return clamp(lightlevel, 0, 255);
}

template<bool doClamp = true>
inline int RescaleLightLevel(int lightlevel) // TODO/tidy: move out of hwrenderer namespace
{
	if constexpr(doClamp)
	{
		//max is needed for negative r_extralight values
		return max(int((clamp(lightlevel, 0, 255) / 255.0) * (255.0 - r_extralight)) + r_extralight, 0);
	}
	else
	{
		//max is needed for negative r_extralight values
		return max(int((max(lightlevel, 0) / 255.0) * (255.0 - r_extralight)) + r_extralight, 0);
	}
}

EXTERN_CVAR(Int, gl_weaponlight);

inline	int getExtraLight()
{
	return r_viewpoint.extralight * gl_weaponlight;
}

