/*
** hw_postprocessshader_ccmds.cpp
**
** Debug ccmds for post-process shaders
**
**---------------------------------------------------------------------------
**
** Copyright 2022-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Copyright 2022 Rachael Alexanderson
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#include "hwrenderer/postprocessing/hw_postprocessshader.h"
#include "hwrenderer/postprocessing/hw_postprocess.h"
#include "printf.h"
#include "c_dispatch.h"

CCMD (shaderenable)
{
	if (argv.argc() < 3)
	{
		Printf("Usage: shaderenable [name] [1/0/-1]\nState '-1' toggles the active shader state\n");
		return;
	}
	auto shaderName = argv[1];

	int value = atoi(argv[2]);

	bool found = 0;
	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name.Compare(shaderName) == 0)
		{
			if (value != -1)
				shader.Enabled = value;
			else
				shader.Enabled = !shader.Enabled; //toggle
			found = 1;
		}
	}
	if (found && value != -1)
		Printf("Changed active state of all instances of %s to %s\n", shaderName, value?"On":"Off");
	else if (found)
		Printf("Toggled active state of all instances of %s\n", shaderName);
	else
		Printf("No shader named '%s' found\n", shaderName);
}

CCMD (shaderuniform)
{
	if (argv.argc() < 3)
	{
		Printf("Usage: shaderuniform [shader name] [uniform name] [[value1 ..]]\n");
		return;
	}
	auto shaderName = argv[1];
	auto uniformName = argv[2];

	bool found = 0;
	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name.Compare(shaderName) == 0)
		{
			if (argv.argc() > 3)
			{
				double *vec4 = shader.Uniforms[uniformName].Values;
				vec4[0] = argv.argc()>=4 ? atof(argv[3]) : 0.0;
				vec4[1] = argv.argc()>=5 ? atof(argv[4]) : 0.0;
				vec4[2] = argv.argc()>=6 ? atof(argv[5]) : 0.0;
				vec4[3] = 1.0;
			}
			else
			{
				double *vec4 = shader.Uniforms[uniformName].Values;
				Printf("Shader '%s' uniform '%s': %f %f %f\n", shaderName, uniformName, vec4[0], vec4[1], vec4[2]);
			}
			found = 1;
		}
	}
	if (found && argv.argc() > 3)
		Printf("Changed uniforms of %s named %s\n", shaderName, uniformName);
	else if (!found)
		Printf("No shader named '%s' found\n", shaderName);
}

CCMD(listshaders)
{
	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		Printf("Shader (%i): %s\n", i, shader.Name.GetChars());
	}
}

CCMD(listuniforms)
{
	if (argv.argc() < 2)
	{
		Printf("Usage: listuniforms [name]\n");
		return;
	}
	auto shaderName = argv[1];

	bool found = 0;
	for (unsigned int i = 0; i < PostProcessShaders.Size(); i++)
	{
		PostProcessShader &shader = PostProcessShaders[i];
		if (shader.Name.Compare(shaderName) == 0)
		{
			Printf("Shader '%s' uniforms:\n", shaderName);

			decltype(shader.Uniforms)::Iterator it(shader.Uniforms);
			decltype(shader.Uniforms)::Pair* pair;

			while (it.NextPair(pair))
			{
				double *vec4 = shader.Uniforms[pair->Key].Values;
				Printf("  %s : %f %f %f\n", pair->Key.GetChars(), vec4[0], vec4[1], vec4[2]);
			}
			found = 1;
		}
	}
	if (!found)
		Printf("No shader named '%s' found\n", shaderName);
}
