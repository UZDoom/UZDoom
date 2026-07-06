#pragma once

#include "vulkaninstance.h"

#include <functional>
#include <mutex>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>

class VulkanSwapChain;
class VulkanSemaphore;
class VulkanFence;
class VulkanPhysicalDevice;
class VulkanSurface;
class VulkanCompatibleDevice;

class VulkanDeviceFaultInfo
{
public:
	std::string description;
	// std::vector<std::string> addressInfos;
	std::vector<std::string> vendorInfos;
};

class VulkanDevice
{
public:
	VulkanDevice(std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanSurface> surface, const VulkanCompatibleDevice& selectedDevice);
	~VulkanDevice();

	std::set<std::string> EnabledDeviceExtensions;
	VulkanDeviceFeatures EnabledFeatures;

	VulkanPhysicalDevice PhysicalDevice;

	std::shared_ptr<VulkanInstance> Instance;
	std::shared_ptr<VulkanSurface> Surface;

	VkDevice device = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;

	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkQueue PresentQueue = VK_NULL_HANDLE;

	int GraphicsFamily = -1;
	int PresentFamily = -1;
	bool GraphicsTimeQueries = false;

	bool SupportsExtension(const char* ext) const;

	void SetObjectName(const char* name, uint64_t handle, VkObjectType type);

	VulkanDeviceFaultInfo GetDeviceFaultInfo();

	inline void CheckVulkanError(VkResult result, const char* text)
	{
		if (result >= VK_SUCCESS) return;
		if (result == VK_ERROR_DEVICE_LOST)
		{
			VulkanDeviceFaultInfo info = GetDeviceFaultInfo();
			if (!info.description.empty())
				VulkanPrintLog("fault", info.description);
			for (const std::string& vendorInfo : info.vendorInfos)
				VulkanPrintLog("fault", vendorInfo);
		}
		VulkanError((text + std::string(": ") + VkResultToString(result)).c_str());
	}

private:
	void CreateDevice();
	void CreateAllocator();
	void ReleaseResources();
};
