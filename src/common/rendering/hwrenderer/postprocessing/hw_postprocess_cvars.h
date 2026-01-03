/*
** hw_postprocess_cvars.h
**
** Postprocessing framework
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Copyright 2016-2020 Magnus Norddahl
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "c_cvars.h"

class IFXAAShader
{
public:
	enum Quality
	{
		None,
		Low,
		Medium,
		High,
		Extreme,
		Count
	};
};



//==========================================================================
//
// CVARs
//
//==========================================================================
EXTERN_CVAR(Bool, gl_bloom)
EXTERN_CVAR(Float, gl_bloom_amount)
EXTERN_CVAR(Float, gl_exposure_scale)
EXTERN_CVAR(Float, gl_exposure_min)
EXTERN_CVAR(Float, gl_exposure_base)
EXTERN_CVAR(Float, gl_exposure_speed)
EXTERN_CVAR(Int, gl_tonemap)
EXTERN_CVAR(Int, gl_bloom_kernel_size)
EXTERN_CVAR(Bool, gl_lens)
EXTERN_CVAR(Float, gl_lens_k)
EXTERN_CVAR(Float, gl_lens_kcube)
EXTERN_CVAR(Float, gl_lens_chromatic)
EXTERN_CVAR(Int, gl_fxaa)
EXTERN_CVAR(Int, gl_ssao)
EXTERN_CVAR(Int, gl_ssao_portals)
EXTERN_CVAR(Float, gl_ssao_strength)
EXTERN_CVAR(Int, gl_ssao_debug)
EXTERN_CVAR(Float, gl_ssao_bias)
EXTERN_CVAR(Float, gl_ssao_radius)
EXTERN_CVAR(Float, gl_ssao_blur)
EXTERN_CVAR(Float, gl_ssao_exponent)
EXTERN_CVAR(Float, gl_paltonemap_powtable)
EXTERN_CVAR(Bool, gl_paltonemap_reverselookup)
EXTERN_CVAR(Float, gl_menu_blur)
EXTERN_CVAR(Float, vid_contrast)
EXTERN_CVAR(Float, vid_saturation)
EXTERN_CVAR(Float, vid_i_blackpoint)
EXTERN_CVAR(Float, vid_i_whitepoint)
EXTERN_CVAR(Int, gl_satformula)
