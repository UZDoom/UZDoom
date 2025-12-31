/*
** vk_pptexture.h
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
#include "vulkan/textures/vk_imagetransition.h"
#include <list>

class VulkanRenderDevice;

class VkPPTexture : public PPTextureBackend
{
public:
	VkPPTexture(VulkanRenderDevice* fb, PPTexture *texture);
	~VkPPTexture();

	void Reset();

	VulkanRenderDevice* fb = nullptr;
	std::list<VkPPTexture*>::iterator it;

	VkTextureImage TexImage;
	std::unique_ptr<VulkanBuffer> Staging;
	VkFormat Format;
};
