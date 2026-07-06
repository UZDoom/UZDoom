
#include "vulkansurface.h"
#include "vulkaninstance.h"

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, VkSurfaceKHR surface) : Instance(std::move(instance)), Surface(surface)
{
}

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(Instance->Instance, Surface, nullptr);
}
