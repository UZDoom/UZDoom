/*
** hw_postprocessshader_ccmds.cpp
**
** Debug ccmds for post-process shaders
**
**---------------------------------------------------------------------------
**
** Copyright 2022-2025 GZDoom Maintainers and Contributors
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
** Copyright 2022 Rachael Alexanderson
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
