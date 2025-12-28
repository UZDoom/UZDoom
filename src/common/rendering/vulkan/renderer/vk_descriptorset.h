/*
** vk_descriptorset.h
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

#include "zvulkan/vulkanobjects.h"
#include <list>
#include "tarray.h"

class VulkanRenderDevice;
class VkMaterial;
class PPTextureInput;
class VkPPRenderPassSetup;

class VkDescriptorSetManager
{
public:
	VkDescriptorSetManager(VulkanRenderDevice* fb);
	~VkDescriptorSetManager();

	void Init();
	void Deinit();
	void BeginFrame();
	void UpdateFixedSet();
	void UpdateHWBufferSet();
	void ResetHWTextureSets();

	VulkanDescriptorSetLayout* GetHWBufferSetLayout() { return HWBufferSetLayout.get(); }
	VulkanDescriptorSetLayout* GetFixedSetLayout() { return FixedSetLayout.get(); }
	VulkanDescriptorSetLayout* GetTextureSetLayout(int numLayers);

	VulkanDescriptorSet* GetHWBufferDescriptorSet() { return HWBufferSet.get(); }
	VulkanDescriptorSet* GetFixedDescriptorSet() { return FixedSet.get(); }
	VulkanDescriptorSet* GetNullTextureDescriptorSet();

	std::unique_ptr<VulkanDescriptorSet> AllocateTextureDescriptorSet(int numLayers);

	VulkanDescriptorSet* GetInput(VkPPRenderPassSetup* passSetup, const TArray<PPTextureInput>& textures, bool bindShadowMapBuffers);

	void AddMaterial(VkMaterial* texture);
	void RemoveMaterial(VkMaterial* texture);

private:
	void CreateHWBufferSetLayout();
	void CreateFixedSetLayout();
	void CreateHWBufferPool();
	void CreateFixedSetPool();

	std::unique_ptr<VulkanDescriptorSet> AllocatePPDescriptorSet(VulkanDescriptorSetLayout* layout);

	VulkanRenderDevice* fb = nullptr;

	std::unique_ptr<VulkanDescriptorSetLayout> HWBufferSetLayout;
	std::unique_ptr<VulkanDescriptorSetLayout> FixedSetLayout;
	std::vector<std::unique_ptr<VulkanDescriptorSetLayout>> TextureSetLayouts;

	std::unique_ptr<VulkanDescriptorPool> HWBufferDescriptorPool;
	std::unique_ptr<VulkanDescriptorPool> FixedDescriptorPool;

	std::unique_ptr<VulkanDescriptorPool> PPDescriptorPool;

	int TextureDescriptorSetsLeft = 0;
	int TextureDescriptorsLeft = 0;
	std::vector<std::unique_ptr<VulkanDescriptorPool>> TextureDescriptorPools;

	std::unique_ptr<VulkanDescriptorSet> HWBufferSet;
	std::unique_ptr<VulkanDescriptorSet> FixedSet;
	std::unique_ptr<VulkanDescriptorSet> NullTextureDescriptorSet;

	std::list<VkMaterial*> Materials;

	static const int maxSets = 10;
};
