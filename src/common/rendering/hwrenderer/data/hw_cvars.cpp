/*
** hw_cvars.cpp
**
** most of the hardware renderer's CVARs.
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2020 Christoph Oelckers
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
#include "c_dispatch.h"
#include "v_video.h"
#include "hw_cvars.h"
#include "menu.h"
#include "printf.h"
#include <algorithm>

CUSTOM_CVAR(Int, gl_fogmode, 2, CVAR_ARCHIVE | CVAR_NOINITCALL)
{
	if (self > 2) self = 2;
	if (self < 0) self = 0;
}

// OpenGL stuff moved here
// GL related CVARs
CVAR(Bool, gl_portals, true, 0)
CVAR(Bool,gl_mirrors,true,0)	// This is for debugging only!
CVAR(Bool,gl_mirror_envmap, true, CVAR_GLOBALCONFIG|CVAR_ARCHIVE)
CVAR(Bool, gl_seamless, true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)

CUSTOM_CVAR(Int, r_mirror_recursions,4,CVAR_GLOBALCONFIG|CVAR_ARCHIVE)
{
	if (self<0) self=0;
	if (self>10) self=10;
}
bool gl_plane_reflection_i;	// This is needed in a header that cannot include the CVAR stuff...
CUSTOM_CVAR(Bool, gl_plane_reflection, true, CVAR_GLOBALCONFIG|CVAR_ARCHIVE)
{
	gl_plane_reflection_i = self;
}

constexpr float GAMMA_DEFAULT = 2.2;
constexpr float GAMMA_HIGH = 3.0;
constexpr float GAMMA_LOW = 0.1;

constexpr float GAMMA_LOW_FIX = (GAMMA_LOW-GAMMA_DEFAULT) / (GAMMA_HIGH-GAMMA_DEFAULT);

CUSTOM_CVARD(Float, vid_gamma, GAMMA_DEFAULT, 0, "(internal) target output gamma")
{
	if (self < GAMMA_LOW) self = GAMMA_LOW;
}

CUSTOM_CVARD(Float, vid_fixgamma, 0.0f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "adjusts gamma component of gamma ramp")
{
	if (self < GAMMA_LOW_FIX) self = GAMMA_LOW_FIX;
	else vid_gamma = self*(GAMMA_HIGH-GAMMA_DEFAULT) + GAMMA_DEFAULT;
}

CUSTOM_CVARD(Float, vid_contrast, 1.f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "adjusts contrast component of gamma ramp")
{
	if (self < 0) self = 0;
	else if (self > 5) self = 5;
}

CUSTOM_CVARD(Float, vid_saturation, 1.f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "adjusts saturation component of gamma ramp")
{
	if (self < -3) self = -3;
	else if (self > 3) self = 3;
}

#ifndef BW_GAP
#define BW_GAP 0.2
#endif

CVAR(Float, vid_i_blackpoint, 1.f, CVAR_VIRTUAL | CVAR_NOINITCALL | CVAR_SYSTEM_ONLY);
CVAR(Float, vid_i_whitepoint, 1.f, CVAR_VIRTUAL | CVAR_NOINITCALL | CVAR_SYSTEM_ONLY);

CUSTOM_CVARD(Float, vid_blackpoint, 0.f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "adjusts what the engine outputs as black")
{
	if (self < 0) self = 0;
	if (self > 1) self = 1;

	float value = self*self;
	float bound = 1 - BW_GAP;
	float buffer = vid_i_whitepoint - BW_GAP;

	vid_i_blackpoint = min(min(buffer, value), bound);
}

CUSTOM_CVARD(Float, vid_whitepoint, 0.f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "adjusts what the engine outputs as white")
{
	if (self < -1) self = -1;
	if (self > 2) self = 2;

	float value = self + 1;
	float bound = 0 + BW_GAP;
	float buffer = vid_i_blackpoint + BW_GAP;
	value = (value*value*value+1)/2;

	vid_i_whitepoint = max(max(buffer, value), bound);
}

#undef BW_GAP

CVAR(Int, gl_satformula, 2, CVAR_ARCHIVE|CVAR_GLOBALCONFIG);

//==========================================================================
//
// Texture CVARs
//
//==========================================================================
CUSTOM_CVARD(Float, gl_texture_filter_anisotropic, 16.f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG | CVAR_NOINITCALL, "changes the OpenGL texture anisotropy setting")
{
	screen->SetTextureFilterMode();
}

CUSTOM_CVARD(Int, gl_texture_filter, 6, CVAR_ARCHIVE|CVAR_GLOBALCONFIG|CVAR_NOINITCALL, "changes the texture filtering settings")
{
	if (self < 0 || self > 6) self=6;
	screen->SetTextureFilterMode();
}

CVAR(Bool, gl_precache, false, CVAR_ARCHIVE)


CUSTOM_CVAR(Int, gl_shadowmap_filter, 1, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
{
	if (self < 0 || self > 8) self = 1;
}
