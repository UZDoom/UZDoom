/*
** vk_renderbuffers.h
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

#include "zvulkan/vulkanobjects.h"
#include "vulkan/textures/vk_imagetransition.h"

class VulkanRenderDevice;
class VkPPRenderPassSetup;
class PPOutput;

enum class WhichDepthStencil {
	None,
	Scene,
	Pipeline,
};

class VkRenderBuffers
{
public:
	VkRenderBuffers(VulkanRenderDevice* fb);
	~VkRenderBuffers();

	void BeginFrame(int width, int height, int sceneWidth, int sceneHeight);

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }
	int GetSceneWidth() const { return mSceneWidth; }
	int GetSceneHeight() const { return mSceneHeight; }
	VkSampleCountFlagBits GetSceneSamples() const { return mSamples; }

	VkTextureImage SceneColor;
	VkTextureImage SceneDepthStencil;
	VkTextureImage SceneNormal;
	VkTextureImage SceneFog;

	VkFormat PipelineDepthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	VkFormat SceneDepthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	VkFormat SceneNormalFormat = VK_FORMAT_A2R10G10B10_UNORM_PACK32;

	static const int NumPipelineImages = 2;
	VkTextureImage PipelineDepthStencil;
	VkTextureImage PipelineImage[NumPipelineImages];

	VulkanFramebuffer* GetOutput(VkPPRenderPassSetup* passSetup, const PPOutput& output, WhichDepthStencil stencilTest, int& framebufferWidth, int& framebufferHeight);

private:
	void CreatePipelineDepthStencil(int width, int height);
	void CreatePipeline(int width, int height);
	void CreateScene(int width, int height, VkSampleCountFlagBits samples);
	void CreateSceneColor(int width, int height, VkSampleCountFlagBits samples);
	void CreateSceneDepthStencil(int width, int height, VkSampleCountFlagBits samples);
	void CreateSceneFog(int width, int height, VkSampleCountFlagBits samples);
	void CreateSceneNormal(int width, int height, VkSampleCountFlagBits samples);
	VkSampleCountFlagBits GetBestSampleCount();

	VulkanRenderDevice* fb = nullptr;

	int mWidth = 0;
	int mHeight = 0;
	int mSceneWidth = 0;
	int mSceneHeight = 0;
	VkSampleCountFlagBits mSamples = VK_SAMPLE_COUNT_1_BIT;
};
