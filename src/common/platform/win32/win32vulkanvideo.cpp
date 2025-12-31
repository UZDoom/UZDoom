/*
** win32vulkanvideo.cpp
**
** Code to let ZDoom draw to the screen
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Magnus Norddahl
** Copyright 2019-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include <assert.h>
#include <algorithm>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <zvulkan/vulkansurface.h>
#include "i_mainwindow.h"

void I_GetVulkanDrawableSize(int *width, int *height)
{
	assert(mainwindow.GetHandle());

	RECT clientRect = { 0 };
	GetClientRect(mainwindow.GetHandle(), &clientRect);

	if (width != nullptr)
	{
		*width = clientRect.right;
	}

	if (height != nullptr)
	{
		*height = clientRect.bottom;
	}
}

bool I_GetVulkanPlatformExtensions(unsigned int *count, const char **names)
{
	static const char* extensions[] = 
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};
	static const unsigned int extensionCount = static_cast<unsigned int>(sizeof extensions / sizeof extensions[0]);

	if (count == nullptr && names == nullptr)
	{
		return false;
	}
	else if (names == nullptr)
	{
		*count = extensionCount;
		return true;
	}
	else
	{
		const bool result = *count >= extensionCount;
		*count = std::min<unsigned int>(*count, extensionCount);

		for (unsigned int i = 0; i < *count; ++i)
		{
			names[i] = extensions[i];
		}

		return result;
	}
}

bool I_CreateVulkanSurface(VkInstance instance, VkSurfaceKHR *surface)
{
	VkWin32SurfaceCreateInfoKHR windowCreateInfo;
	windowCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	windowCreateInfo.pNext = nullptr;
	windowCreateInfo.flags = 0;
	windowCreateInfo.hwnd = mainwindow.GetHandle();
	windowCreateInfo.hinstance = GetModuleHandle(nullptr);

	const VkResult result = vkCreateWin32SurfaceKHR(instance, &windowCreateInfo, nullptr, surface);
	return result == VK_SUCCESS;
}
