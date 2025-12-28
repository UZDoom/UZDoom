/*
** vk_framebuffer.h
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
#include <array>
#include <map>
#include <vector>

class VulkanRenderDevice;
enum class PPFilterMode;
enum class PPWrapMode;

class VkFramebufferManager
{
public:
	VkFramebufferManager(VulkanRenderDevice* fb);
	~VkFramebufferManager();

	void AcquireImage();
	void QueuePresent();

	std::map<int, std::unique_ptr<VulkanFramebuffer>> Framebuffers;

	std::shared_ptr<VulkanSwapChain> SwapChain;
	int PresentImageIndex = -1;

	std::unique_ptr<VulkanSemaphore> SwapChainImageAvailableSemaphore;
	std::vector<std::unique_ptr<VulkanSemaphore>> RenderFinishedSemaphores;

private:
	VulkanRenderDevice* fb = nullptr;
	int CurrentWidth = 0;
	int CurrentHeight = 0;
	bool CurrentVSync = false;
	bool CurrentHdr = false;
	bool CurrentExclusiveFullscreen = false;
};
