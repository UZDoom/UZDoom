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

