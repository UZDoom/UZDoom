/*
** vk_renderstate.h
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

#include "vulkan/system/vk_hwbuffer.h"
#include "vulkan/shaders/vk_shader.h"
#include "vulkan/renderer/vk_renderpass.h"
#include "vulkan/renderer/vk_streambuffer.h"

#include "name.h"

#include "hw_renderstate.h"
#include "hw_material.h"

class VulkanRenderDevice;
class VkRenderPassSetup;
class VkTextureImage;

class VkRenderState : public FRenderState
{
public:
	VkRenderState(VulkanRenderDevice* fb);
	virtual ~VkRenderState() = default;

	// Draw commands
	void ClearScreen() override;
	void Draw(int dt, int index, int count, bool apply = true) override;
	void DrawIndexed(int dt, int index, int count, bool apply = true) override;

	// Immediate render state change commands. These only change infrequently and should not clutter the render state.
	bool SetDepthClamp(bool on) override;
	void SetDepthMask(bool on) override;
	void SetDepthFunc(int func) override;
	void SetDepthRange(float min, float max) override;
	void SetColorMask(bool r, bool g, bool b, bool a) override;
	void SetStencil(int offs, int op, int flags = -1) override;
	void SetCulling(int mode) override;
	void EnableClipDistance(int num, bool state) override;
	void Clear(int targets) override;
	void EnableStencil(bool on) override;
	void SetScissor(int x, int y, int w, int h) override;
	void SetViewport(int x, int y, int w, int h) override;
	void EnableDepthTest(bool on) override;
	void EnableMultisampling(bool on) override;
	void EnableLineSmooth(bool on) override;
	void EnableDrawBuffers(int count, bool apply) override;

	void BeginFrame();
	void SetRenderTarget(VkTextureImage *image, VulkanImageView *depthStencilView, int width, int height, VkFormat Format, VkSampleCountFlagBits samples);
	void Bind(int bindingpoint, uint32_t offset);
	void EndRenderPass();
	void EndFrame();

protected:
	void Apply(int dt);
	void ApplyRenderPass(int dt);
	void ApplyStencilRef();
	void ApplyDepthBias();
	void ApplyScissor();
	void ApplyViewport();
	void ApplyStreamData();
	void ApplyMatrices();
	void ApplyPushConstants();
	void ApplyHWBufferSet();
	void ApplyVertexBuffers();
	void ApplyMaterial();

	void BeginRenderPass(VulkanCommandBuffer *cmdbuffer);
	void WaitForStreamBuffers();

	VulkanRenderDevice* fb = nullptr;

	bool mDepthClamp = true;
	VulkanCommandBuffer *mCommandBuffer = nullptr;
	VkPipelineKey mPipelineKey = {};
	VkRenderPassSetup *mPassSetup = nullptr;
	int mClearTargets = 0;
	bool mNeedApply = true;

	int mScissorX = 0, mScissorY = 0, mScissorWidth = -1, mScissorHeight = -1;
	int mViewportX = 0, mViewportY = 0, mViewportWidth = -1, mViewportHeight = -1;
	float mViewportDepthMin = 0.0f, mViewportDepthMax = 1.0f;
	bool mScissorChanged = true;
	bool mViewportChanged = true;

	bool mDepthTest = false;
	bool mDepthWrite = false;
	bool mStencilTest = false;

	bool mStencilRefChanged = false;
	int mStencilRef = 0;
	int mStencilOp = 0;
	int mDepthFunc = 0;
	int mColorMask = 15;
	int mCullMode = 0;

	PushConstants mPushConstants = {};

	uint32_t mLastViewpointOffset = 0xffffffff;
	uint32_t mLastMatricesOffset = 0xffffffff;
	uint32_t mLastStreamDataOffset = 0xffffffff;
	uint32_t mViewpointOffset = 0;

	VkStreamBufferWriter mStreamBufferWriter;
	VkMatrixBufferWriter mMatrixBufferWriter;

	int mLastVertexOffsets[2] = { 0, 0 };
	IVertexBuffer *mLastVertexBuffer = nullptr;
	IIndexBuffer *mLastIndexBuffer = nullptr;

	bool mLastModelMatrixEnabled = true;
	bool mLastTextureMatrixEnabled = true;

	int mApplyCount = 0;

	struct RenderTarget
	{
		VkTextureImage *Image = nullptr;
		VulkanImageView *DepthStencil = nullptr;
		int Width = 0;
		int Height = 0;
		VkFormat Format = VK_FORMAT_R16G16B16A16_SFLOAT;
		VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
		int DrawBuffers = 1;
	} mRenderTarget;
};

class VkRenderStateMolten : public VkRenderState
{
public:
	using VkRenderState::VkRenderState;

	void Draw(int dt, int index, int count, bool apply = true) override;
};
