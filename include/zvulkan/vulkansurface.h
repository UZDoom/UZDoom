#pragma once

#include "vulkaninstance.h"

class VulkanSurface
{
public:
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, VkSurfaceKHR surface);
	~VulkanSurface();

	std::shared_ptr<VulkanInstance> Instance;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
};
