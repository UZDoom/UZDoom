/*
** vk_ppshader.cpp
**
** Vulkan backend
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
