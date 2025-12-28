/*
** hw_postprocess_cvars.h
**
** Postprocessing framework
**
**---------------------------------------------------------------------------
**
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
** Copyright 2016-2020 Magnus Norddahl
**
** This software is provided 'as-is', without any express or implied
** warranty.  In no event will the authors be held liable for any damages
** arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose,
** including commercial applications, and to alter it and redistribute it
** freely, subject to the following restrictions:
**
** 1. The origin of this software must not be misrepresented; you must not
**    claim that you wrote the original software. If you use this software
**    in a product, an acknowledgment in the product documentation would be
**    appreciated but is not required.
** 2. Altered source versions must be plainly marked as such, and must not be
**    misrepresented as being the original software.
** 3. This notice may not be removed or altered from any source distribution.
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
EXTERN_CVAR(Float, vid_brightness)
EXTERN_CVAR(Float, vid_contrast)
EXTERN_CVAR(Float, vid_saturation)
EXTERN_CVAR(Float, vid_blackpoint)
EXTERN_CVAR(Float, vid_whitepoint)
EXTERN_CVAR(Int, gl_satformula)

