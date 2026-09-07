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
#include <utility>
#include "shaderuniforms.h"

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

namespace ShaderInputsOutputs
{
	enum ShaderProperty
	{
		GBufferPass = 1,
		EffectShader = 2,
		HasClipDistance = 4,
		Simple = 8,
	};

	enum class ShaderPosition
	{
		VInput, // vertex input
		VOutput, // vertex output/frag input
		FOutput, // frag output
	};

	struct ShaderIOEntry
	{
		ShaderPosition position;
		VaryingFieldDesc field;
		int requiredProperties;
		int forbiddenProperties;
	};

	extern int ShaderProperties[ALLSHADER_COUNT];
	extern TArray<ShaderIOEntry> ShaderFields;

	//varying list must match between the frag and vertex shader of the same program
	FString GenerateInputsOutputs(bool isVulkan, bool isFrag, int flags);

	inline FString GenerateInputsOutputs(bool isVulkan, bool isFrag, AllShaderIndex type, bool isGBuffer, bool hasClipDistance)
	{
		return GenerateInputsOutputs(isVulkan, isFrag,
			ShaderProperties[static_cast<int>(type)] | (isGBuffer ? GBufferPass : 0) | (hasClipDistance ? HasClipDistance : 0));
	}
}
