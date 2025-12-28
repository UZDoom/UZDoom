/*
** vk_ppshader.cpp
**
** Vulkan backend
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

#include "vk_ppshader.h"
#include "vk_shader.h"
#include "vulkan/system/vk_renderdevice.h"
#include "zvulkan/vulkanbuilders.h"
#include "vulkan/system/vk_commandbuffer.h"
#include "filesystem.h"
#include "cmdlib.h"

VkPPShader::VkPPShader(VulkanRenderDevice* fb, PPShader *shader) : fb(fb)
{
	FString prolog;
	if (!shader->Uniforms.empty())
		prolog = UniformBlockDecl::Create("Uniforms", shader->Uniforms, -1);

	// Add automatic uniforms in separate block at AUTOMATIC_UNIFORMS_BINDING
	prolog.AppendFormat("layout(set = 0, binding = %d) uniform AutomaticUniforms {\n", AUTOMATIC_UNIFORMS_BINDING);
	prolog += "    float InputTimeDelta;\n";
	prolog += "    float InputTime;\n";
	prolog += "    float InputTimeGame;\n";
	prolog += "};\n";

	prolog += shader->Defines;

	VertexShader = ShaderBuilder()
		.Type(ShaderType::Vertex)
		.AddSource(shader->VertexShader.GetChars(), LoadShaderCode(shader->VertexShader, "", shader->Version).GetChars())
		.DebugName(shader->VertexShader.GetChars())
		.OnIncludeLocal(VkShaderManager::OnInclude)
		.OnIncludeSystem(VkShaderManager::OnInclude)
		.Create(shader->VertexShader.GetChars(), fb->device.get());

	FragmentShader = ShaderBuilder()
		.Type(ShaderType::Fragment)
		.AddSource(shader->FragmentShader.GetChars(), LoadShaderCode(shader->FragmentShader, prolog, shader->Version).GetChars())
		.DebugName(shader->FragmentShader.GetChars())
		.OnIncludeLocal(VkShaderManager::OnInclude)
		.OnIncludeSystem(VkShaderManager::OnInclude)
		.Create(shader->FragmentShader.GetChars(), fb->device.get());

	fb->GetShaderManager()->AddVkPPShader(this);
}

VkPPShader::~VkPPShader()
{
	if (fb)
		fb->GetShaderManager()->RemoveVkPPShader(this);
}

void VkPPShader::Reset()
{
	if (fb)
	{
		fb->GetCommands()->DrawDeleteList->Add(std::move(VertexShader));
		fb->GetCommands()->DrawDeleteList->Add(std::move(FragmentShader));
	}
}

FString VkPPShader::LoadShaderCode(const FString &lumpName, const FString &defines, int version)
{
	int lump = fileSystem.CheckNumForFullName(lumpName.GetChars());
	if (lump == -1) I_FatalError("Unable to load '%s'", lumpName.GetChars());
	FString code = GetStringFromLump(lump);

	FString patchedCode;
	patchedCode.AppendFormat("#version %d\n#extension GL_GOOGLE_include_directive : enable\n", 450);
	patchedCode << defines;
	patchedCode << "#line 1\n";
	patchedCode << code;
	return patchedCode;
}
