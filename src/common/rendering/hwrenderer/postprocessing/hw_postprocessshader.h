/*
** hw_postprocessshader.h
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

#include "zstring.h"
#include "tarray.h"

enum class PixelFormat
{
	Rgba8,
	Rgba16f,
	R32f,
	Rg16f,
	Rgba16_snorm
};

enum class PostProcessUniformType
{
	Undefined,
	Int,
	Float,
	Vec2,
	Vec3,
	Vec4
};

struct PostProcessUniformValue
{
	PostProcessUniformType Type = PostProcessUniformType::Undefined;
	double Values[4] = { 0.0, 0.0, 0.0, 0.0 };
};

struct PostProcessShader
{
	FString Target;
	FString ShaderLumpName;
	int ShaderVersion = 0;

	FString Name;
	bool Enabled = false;

	TMap<FString, PostProcessUniformValue> Uniforms;
	TMap<FString, FString> Textures;
};

extern TArray<PostProcessShader> PostProcessShaders;

