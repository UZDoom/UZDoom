/*
** win32vulkanvideo.h
**
** Code to let ZDoom draw to the screen
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Magnus Norddahl
** Copyright 2019-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "win32basevideo.h"
#include "c_cvars.h"
#include "vulkan/system/vk_renderdevice.h"
#include <zvulkan/vulkansurface.h>
#include <zvulkan/vulkanbuilders.h>

void I_GetVulkanDrawableSize(int* width, int* height);
bool I_CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface);
bool I_GetVulkanPlatformExtensions(unsigned int* count, const char** names);

EXTERN_CVAR(Bool, vid_fullscreen)
EXTERN_CVAR(Bool, vk_debug)
EXTERN_CVAR(Int, vk_device)

//==========================================================================
//
// 
//
//==========================================================================

class Win32VulkanVideo : public Win32BaseVideo
{
	std::shared_ptr<VulkanSurface> surface;
public:
	Win32VulkanVideo() 
	{
		unsigned int count = 64;
		const char* names[64];
		if (!I_GetVulkanPlatformExtensions(&count, names))
			VulkanError("I_GetVulkanPlatformExtensions failed");

		VulkanInstanceBuilder builder;
		builder.DebugLayer(vk_debug);
		for (unsigned int i = 0; i < count; i++)
			builder.RequireExtension(names[i]);
		auto instance = builder.Create();

		VkSurfaceKHR surfacehandle = nullptr;
		if (!I_CreateVulkanSurface(instance->Instance, &surfacehandle))
			VulkanError("I_CreateVulkanSurface failed");

		surface = std::make_shared<VulkanSurface>(instance, surfacehandle);
	}

	~Win32VulkanVideo()
	{
	}

	void Shutdown() override
	{
		surface.reset();
	}

	DFrameBuffer *CreateFrameBuffer() override
	{
		auto fb = new VulkanRenderDevice(m_hMonitor, vid_fullscreen, surface);
		return fb;
	}

protected:
};
