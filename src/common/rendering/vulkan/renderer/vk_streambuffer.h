/*
** vk_streambuffer.h
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

#include "vulkan/system/vk_hwbuffer.h"
#include "vulkan/shaders/vk_shader.h"

class VkStreamBuffer;
class VkMatrixBuffer;

class VkStreamBufferWriter
{
public:
	VkStreamBufferWriter(VulkanRenderDevice* fb);

	bool Write(const StreamData& data);
	void Reset();

	uint32_t DataIndex() const { return mDataIndex; }
	uint32_t StreamDataOffset() const { return mStreamDataOffset; }

private:
	VkStreamBuffer* mBuffer;
	uint32_t mDataIndex = MAX_STREAM_DATA - 1;
	uint32_t mStreamDataOffset = 0;
};

class VkMatrixBufferWriter
{
public:
	VkMatrixBufferWriter(VulkanRenderDevice* fb);

	bool Write(const VSMatrix& modelMatrix, bool modelMatrixEnabled, const VSMatrix& textureMatrix, bool textureMatrixEnabled);
	void Reset();

	uint32_t Offset() const { return mOffset; }

private:
	VkStreamBuffer* mBuffer;
	MatricesUBO mMatrices = {};
	VSMatrix mIdentityMatrix;
	uint32_t mOffset = 0;
};
