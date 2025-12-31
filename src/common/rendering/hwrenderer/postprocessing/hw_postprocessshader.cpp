/*
** hw_postprocessshader.cpp
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

#include "vm.h"
#include "hwrenderer/postprocessing/hw_postprocessshader.h"
#include "hwrenderer/postprocessing/hw_postprocess.h"

static void ShaderSetEnabled(const FString &shaderName, bool value)
{
	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name == shaderName)
			shader.Enabled = value;
	}
}

DEFINE_ACTION_FUNCTION_NATIVE(_PPShader, SetEnabled, ShaderSetEnabled)
{
	PARAM_PROLOGUE;
	PARAM_STRING(shaderName);
	PARAM_BOOL(value);
	ShaderSetEnabled(shaderName, value);

	return 0;
}

static void ShaderSetUniform1f(const FString &shaderName, const FString &uniformName, double value)
{
	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name == shaderName)
		{
			double *vec4 = shader.Uniforms[uniformName].Values;
			vec4[0] = value;
			vec4[1] = 0.0;
			vec4[2] = 0.0;
			vec4[3] = 1.0;
		}
	}
}

DEFINE_ACTION_FUNCTION_NATIVE(_PPShader, SetUniform1f, ShaderSetUniform1f)
{
	PARAM_PROLOGUE;
	PARAM_STRING(shaderName);
	PARAM_STRING(uniformName);
	PARAM_FLOAT(value);
	ShaderSetUniform1f(shaderName, uniformName, value);
	return 0;
}

DEFINE_ACTION_FUNCTION(_PPShader, SetUniform2f)
{
	PARAM_PROLOGUE;
	PARAM_STRING(shaderName);
	PARAM_STRING(uniformName);
	PARAM_FLOAT(x);
	PARAM_FLOAT(y);

	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name == shaderName)
		{
			double *vec4 = shader.Uniforms[uniformName].Values;
			vec4[0] = x;
			vec4[1] = y;
			vec4[2] = 0.0;
			vec4[3] = 1.0;
		}
	}
	return 0;
}

DEFINE_ACTION_FUNCTION(_PPShader, SetUniform3f)
{
	PARAM_PROLOGUE;
	PARAM_STRING(shaderName);
	PARAM_STRING(uniformName);
	PARAM_FLOAT(x);
	PARAM_FLOAT(y);
	PARAM_FLOAT(z);

	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name == shaderName)
		{
			double *vec4 = shader.Uniforms[uniformName].Values;
			vec4[0] = x;
			vec4[1] = y;
			vec4[2] = z;
			vec4[3] = 1.0;
		}
	}
	return 0;
}

DEFINE_ACTION_FUNCTION(_PPShader, SetUniform4f)
{
	PARAM_PROLOGUE;
	PARAM_STRING(shaderName);
	PARAM_STRING(uniformName);
	PARAM_FLOAT(x);
	PARAM_FLOAT(y);
	PARAM_FLOAT(z);
	PARAM_FLOAT(w);

	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name == shaderName)
		{
			double *vec4 = shader.Uniforms[uniformName].Values;
			vec4[0] = x;
			vec4[1] = y;
			vec4[2] = z;
			vec4[3] = w;
		}
	}
	return 0;
}

DEFINE_ACTION_FUNCTION(_PPShader, SetUniform1i)
{
	PARAM_PROLOGUE;
	PARAM_STRING(shaderName);
	PARAM_STRING(uniformName);
	PARAM_INT(value);

	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name == shaderName)
		{
			double *vec4 = shader.Uniforms[uniformName].Values;
			vec4[0] = (double)value;
			vec4[1] = 0.0;
			vec4[2] = 0.0;
			vec4[3] = 1.0;
		}
	}
	return 0;
}
