/*
** vk_hwtexture.h
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

#pragma once

#ifdef LoadImage
#undef LoadImage
#endif

#define SHADED_TEXTURE -1
#define DIRECT_PALETTE -2

#include "tarray.h"
#include "hw_ihwtexture.h"
#include <zvulkan/vulkanobjects.h>
#include "vk_imagetransition.h"
#include "hw_material.h"
#include <list>

struct FMaterialState;
class VulkanDescriptorSet;
class VulkanImage;
class VulkanImageView;
class VulkanBuffer;
class VulkanRenderDevice;
class FGameTexture;

class VkHardwareTexture : public IHardwareTexture
{
	friend class VkMaterial;
public:
	VkHardwareTexture(VulkanRenderDevice* fb, int numchannels);
	~VkHardwareTexture();

	void Reset();

	// Software renderer stuff
	void AllocateBuffer(int w, int h, int texelsize) override;
	uint8_t *MapBuffer() override;
	unsigned int CreateTexture(unsigned char * buffer, int w, int h, int texunit, bool mipmap, const char *name) override;

	// Wipe screen
	void CreateWipeTexture(int w, int h, const char *name);

	VkTextureImage *GetImage(FTexture *tex, int translation, int flags);
	VkTextureImage *GetDepthStencil(FTexture *tex);

	VulkanRenderDevice* fb = nullptr;
	std::list<VkHardwareTexture*>::iterator it;

private:
	void CreateImage(FTexture *tex, int translation, int flags);

	void CreateTexture(int w, int h, int pixelsize, VkFormat format, const void *pixels, bool mipmap);
	static int GetMipLevels(int w, int h);

	VkTextureImage mImage;
	int mTexelsize = 4;

	VkTextureImage mDepthStencil;

	uint8_t* mappedSWFB = nullptr;
};

class VkMaterial : public FMaterial
{
public:
	VkMaterial(VulkanRenderDevice* fb, FGameTexture* tex, int scaleflags);
	~VkMaterial();

	VulkanDescriptorSet* GetDescriptorSet(const FMaterialState& state);

	void DeleteDescriptors() override;

	VulkanRenderDevice* fb = nullptr;
	std::list<VkMaterial*>::iterator it;

private:
	struct DescriptorEntry
	{
		int clampmode;
		intptr_t remap;
		std::unique_ptr<VulkanDescriptorSet> descriptor;

		DescriptorEntry(int cm, intptr_t f, std::unique_ptr<VulkanDescriptorSet>&& d)
		{
			clampmode = cm;
			remap = f;
			descriptor = std::move(d);
		}
	};

	std::vector<DescriptorEntry> mDescriptorSets;
};
