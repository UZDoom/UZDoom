/*
** hw_cvars.h
**
** most of the hardware renderer's CVARs.
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once


#include "c_cvars.h"

EXTERN_CVAR(Bool,gl_enhanced_nightvision)
EXTERN_CVAR(Int, screenblocks);
EXTERN_CVAR(Int, gl_texture_filter)
EXTERN_CVAR(Float, gl_texture_filter_anisotropic)
EXTERN_CVAR(Int, gl_texture_format)
EXTERN_CVAR(Bool, gl_usefb)

EXTERN_CVAR(Int, gl_weaponlight)

EXTERN_CVAR (Bool, gl_light_sprites);
EXTERN_CVAR (Bool, gl_light_particles);
EXTERN_CVAR (Bool, gl_light_shadowmap);
EXTERN_CVAR (Int, gl_shadowmap_quality);

EXTERN_CVAR(Int, gl_fogmode)
EXTERN_CVAR(Bool,gl_mirror_envmap)

EXTERN_CVAR(Bool,gl_mirrors)
EXTERN_CVAR(Bool,gl_mirror_envmap)
EXTERN_CVAR(Bool, gl_seamless)

EXTERN_CVAR(Float, gl_mask_threshold)
EXTERN_CVAR(Float, gl_mask_sprite_threshold)

EXTERN_CVAR(Int, gl_multisample)

EXTERN_CVAR(Bool, gl_bloom)
EXTERN_CVAR(Float, gl_bloom_amount)
EXTERN_CVAR(Int, gl_bloom_kernel_size)
EXTERN_CVAR(Int, gl_tonemap)
EXTERN_CVAR(Float, gl_exposure)
EXTERN_CVAR(Bool, gl_lens)
EXTERN_CVAR(Float, gl_lens_k)
EXTERN_CVAR(Float, gl_lens_kcube)
EXTERN_CVAR(Float, gl_lens_chromatic)
EXTERN_CVAR(Int, gl_ssao)
EXTERN_CVAR(Int, gl_ssao_portals)
EXTERN_CVAR(Float, gl_ssao_strength)
EXTERN_CVAR(Int, gl_ssao_debug)
EXTERN_CVAR(Float, gl_ssao_bias)
EXTERN_CVAR(Float, gl_ssao_radius)
EXTERN_CVAR(Float, gl_ssao_blur_amount)

EXTERN_CVAR(Int, gl_debug_level)
EXTERN_CVAR(Bool, gl_debug_breakpoint)

EXTERN_CVAR(Int, gl_shadowmap_filter)

EXTERN_CVAR(Bool, gl_brightfog)
EXTERN_CVAR(Bool, gl_lightadditivesurfaces)
EXTERN_CVAR(Bool, gl_notexturefill)

EXTERN_CVAR(Bool, r_radarclipper)
EXTERN_CVAR(Bool, r_dithertransparency)
