/*
** vk_pprenderstate.h
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

class VkPPRenderPassSetup;
class VkPPShader;
class VkPPTexture;
class VkTextureImage;
class VulkanRenderDevice;

class VkPPRenderState : public PPRenderState
{
public:
	VkPPRenderState(VulkanRenderDevice* fb);

	void PushGroup(const FString &name) override;
	void PopGroup() override;

	void Draw() override;
	void CopyToTexture(PPTexture* dst) override;

private:
	void RenderScreenQuad(VkPPRenderPassSetup *passSetup, VulkanDescriptorSet *descriptorSet, VulkanFramebuffer *framebuffer, int framebufferWidth, int framebufferHeight, int x, int y, int width, int height, const void *pushConstants, uint32_t pushConstantsSize, bool stencilTest);

	VulkanRenderDevice* fb = nullptr;
};
