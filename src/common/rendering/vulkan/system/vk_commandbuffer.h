/*
** vk_commandbuffer.h
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

#include <zvulkan/vulkandevice.h>
#include <zvulkan/vulkanobjects.h>
#include "zstring.h"

class VulkanRenderDevice;

class VkCommandBufferManager
{
public:
	VkCommandBufferManager(VulkanRenderDevice* fb);
	~VkCommandBufferManager();

	void BeginFrame();

	VulkanCommandBuffer* GetTransferCommands();
	VulkanCommandBuffer* GetDrawCommands();

	void FlushCommands(bool finish, bool lastsubmit = false, bool uploadOnly = false);

	void WaitForCommands(bool finish) { WaitForCommands(finish, false); }
	void WaitForCommands(bool finish, bool uploadOnly);

	void PushGroup(const FString& name);
	void PopGroup();
	void UpdateGpuStats();

	class DeleteList
	{
	public:
		std::vector<std::unique_ptr<VulkanBuffer>> Buffers;
		std::vector<std::unique_ptr<VulkanSampler>> Samplers;
		std::vector<std::unique_ptr<VulkanImage>> Images;
		std::vector<std::unique_ptr<VulkanImageView>> ImageViews;
		std::vector<std::unique_ptr<VulkanFramebuffer>> Framebuffers;
		std::vector<std::unique_ptr<VulkanAccelerationStructure>> AccelStructs;
		std::vector<std::unique_ptr<VulkanDescriptorPool>> DescriptorPools;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> Descriptors;
		std::vector<std::unique_ptr<VulkanShader>> Shaders;
		std::vector<std::unique_ptr<VulkanCommandBuffer>> CommandBuffers;
		size_t TotalSize = 0;

		void Add(std::unique_ptr<VulkanBuffer> obj) { if (obj) { TotalSize += obj->size; Buffers.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanSampler> obj) { if (obj) { Samplers.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanImage> obj) { if (obj) { Images.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanImageView> obj) { if (obj) { ImageViews.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanFramebuffer> obj) { if (obj) { Framebuffers.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanAccelerationStructure> obj) { if (obj) { AccelStructs.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanDescriptorPool> obj) { if (obj) { DescriptorPools.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanDescriptorSet> obj) { if (obj) { Descriptors.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanCommandBuffer> obj) { if (obj) { CommandBuffers.push_back(std::move(obj)); } }
		void Add(std::unique_ptr<VulkanShader> obj) { if (obj) { Shaders.push_back(std::move(obj)); } }
	};

	std::unique_ptr<DeleteList> TransferDeleteList = std::make_unique<DeleteList>();
	std::unique_ptr<DeleteList> DrawDeleteList = std::make_unique<DeleteList>();

	void DeleteFrameObjects(bool uploadOnly = false);

private:
	void FlushCommands(VulkanCommandBuffer** commands, size_t count, bool finish, bool lastsubmit);

	VulkanRenderDevice* fb = nullptr;

	std::unique_ptr<VulkanCommandPool> mCommandPool;

	std::unique_ptr<VulkanCommandBuffer> mTransferCommands;
	std::unique_ptr<VulkanCommandBuffer> mDrawCommands;

	enum { maxConcurrentSubmitCount = 8 };
	std::unique_ptr<VulkanSemaphore> mSubmitSemaphore[maxConcurrentSubmitCount];
	std::unique_ptr<VulkanFence> mSubmitFence[maxConcurrentSubmitCount];
	VkFence mSubmitWaitFences[maxConcurrentSubmitCount];
	int mNextSubmit = 0;

	struct TimestampQuery
	{
		FString name;
		uint32_t startIndex;
		uint32_t endIndex;
	};

	enum { MaxTimestampQueries = 100 };
	std::unique_ptr<VulkanQueryPool> mTimestampQueryPool;
	int mNextTimestampQuery = 0;
	std::vector<size_t> mGroupStack;
	std::vector<TimestampQuery> timeElapsedQueries;
};
