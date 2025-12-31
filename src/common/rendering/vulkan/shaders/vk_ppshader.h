/*
** vk_ppshader.h
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

#pragma once

#include "hwrenderer/postprocessing/hw_postprocess.h"
#include <zvulkan/vulkanobjects.h>
#include <list>

class VulkanRenderDevice;

class VkPPShader : public PPShaderBackend
{
public:
	VkPPShader(VulkanRenderDevice* fb, PPShader *shader);
	~VkPPShader();

	void Reset();

	VulkanRenderDevice* fb = nullptr;
	std::list<VkPPShader*>::iterator it;

	std::unique_ptr<VulkanShader> VertexShader;
	std::unique_ptr<VulkanShader> FragmentShader;

private:
	FString LoadShaderCode(const FString &lumpname, const FString &defines, int version);
};
