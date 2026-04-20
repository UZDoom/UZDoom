/*
** hw_shaderpatcher.h
**
** Modifies shader source to account for different syntax versions or engine changes.
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2018 Christoph Oelckers
** Copyright 2016-2018 Magnus Norddahl
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

#pragma once

#include "tarray.h"
#include "zstring.h"
#include "utility"

FString RemoveLegacyUserUniforms(FString code);
FString RemoveSamplerBindings(FString code, TArray<std::pair<FString, int>> &samplerstobind);	// For GL 3.3 compatibility which cannot declare sampler bindings in the sampler source.
FString RemoveLayoutLocationDecl(FString code, const char *inoutkeyword);

struct FDefaultShader
{
	const char * ShaderName;
	const char * gettexelfunc;
	const char * lightfunc;
	const char * Defines;
};

struct FEffectShader
{
	const char *ShaderName;
	const char *vp;
	const char *fp1;
	const char *fp2;
	const char *fp3;
	const char *defines;
};

extern const FDefaultShader defaultshaders[];
extern const FEffectShader effectshaders[];

