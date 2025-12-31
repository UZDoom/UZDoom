/*
** vk_hwbuffer.h
**
** Vulkan backend
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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

#include "hwrenderer/data/buffers.h"
#include "zvulkan/vulkanobjects.h"
#include "tarray.h"
#include <list>

#ifdef _MSC_VER
// silence bogus warning C4250: 'VkHardwareVertexBuffer': inherits 'VkHardwareBuffer::VkHardwareBuffer::SetData' via dominance
// According to internet infos, the warning is erroneously emitted in this case.
#pragma warning(disable:4250) 
#endif

class VulkanRenderDevice;

class VkHardwareBuffer : virtual public IBuffer
{
public:
	VkHardwareBuffer(VulkanRenderDevice* fb);
	~VkHardwareBuffer();

	void Reset();

	void SetData(size_t size, const void *data, BufferUsageType usage) override;
	void SetSubData(size_t offset, size_t size, const void *data) override;
	void Resize(size_t newsize) override;

	void Map() override;
	void Unmap() override;

	void *Lock(unsigned int size) override;
	void Unlock() override;

	VulkanRenderDevice* fb = nullptr;
	std::list<VkHardwareBuffer*>::iterator it;

	VkBufferUsageFlags mBufferType = 0;
	std::unique_ptr<VulkanBuffer> mBuffer;
	std::unique_ptr<VulkanBuffer> mStaging;
	bool mPersistent = false;
	TArray<uint8_t> mStaticUpload;
};

class VkHardwareVertexBuffer : public IVertexBuffer, public VkHardwareBuffer
{
public:
	VkHardwareVertexBuffer(VulkanRenderDevice* fb) : VkHardwareBuffer(fb) { mBufferType = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
	void SetFormat(int numBindingPoints, int numAttributes, size_t stride, const FVertexBufferAttribute *attrs) override;

	int VertexFormat = -1;
};

class VkHardwareIndexBuffer : public IIndexBuffer, public VkHardwareBuffer
{
public:
	VkHardwareIndexBuffer(VulkanRenderDevice* fb) : VkHardwareBuffer(fb) { mBufferType = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
};

class VkHardwareDataBuffer : public IDataBuffer, public VkHardwareBuffer
{
public:
	VkHardwareDataBuffer(VulkanRenderDevice* fb, int bindingpoint, bool ssbo, bool needresize) : VkHardwareBuffer(fb), bindingpoint(bindingpoint)
	{
		mBufferType = ssbo ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (needresize)
		{
			mBufferType |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
	}

	void BindRange(FRenderState *state, size_t start, size_t length) override;

	int bindingpoint;
};
