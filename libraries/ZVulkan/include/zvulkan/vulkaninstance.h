#pragma once

#include "vulkan.h"
#include "volk/volk.h"
#include "vk_mem_alloc/vk_mem_alloc.h"

#if defined(_WIN32)
#undef min
#undef max
#endif

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <set>

std::string VkResultToString(VkResult result);

void VulkanPrintLog(const char* typestr, const std::string& msg);
void VulkanError(const char* text);

class VulkanDeviceFeatures
{
public:
	VkPhysicalDeviceFeatures Features = {};
	VkPhysicalDeviceBufferDeviceAddressFeatures BufferDeviceAddress = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
	VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructure = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	VkPhysicalDeviceRayQueryFeaturesKHR RayQuery = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
	VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexing = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
	VkPhysicalDeviceFaultFeaturesEXT Fault = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT };
	VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT GraphicsPipelineLibrary = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT };
};

class VulkanDeviceProperties
{
public:
	VkPhysicalDeviceProperties Properties = {};
	VkPhysicalDeviceMemoryProperties Memory = {};
	VkPhysicalDeviceAccelerationStructurePropertiesKHR AccelerationStructure = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR };
	VkPhysicalDeviceDescriptorIndexingProperties DescriptorIndexing = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT };
	VkPhysicalDeviceLayeredDriverPropertiesMSFT LayeredDriver = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT };
	VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT GraphicsPipelineLibrary = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT };
};

class VulkanPhysicalDevice
{
public:
	VkPhysicalDevice Device = VK_NULL_HANDLE;
	std::vector<VkExtensionProperties> Extensions;
	std::vector<VkQueueFamilyProperties> QueueFamilies;
	VulkanDeviceProperties Properties;
	VulkanDeviceFeatures Features;
};

class VulkanInstance
{
public:
	VulkanInstance(std::vector<uint32_t> apiVersionsToTry, std::set<std::string> requiredExtensions, std::set<std::string> optionalExtensions, bool wantDebugLayer);
	~VulkanInstance();

	std::vector<uint32_t> ApiVersionsToTry;

	std::set<std::string> RequiredExtensions;
	std::set<std::string> OptionalExtensions;

	std::vector<VkLayerProperties> AvailableLayers;
	std::vector<VkExtensionProperties> AvailableExtensions;

	std::set<std::string> EnabledLayers;
	std::set<std::string> EnabledExtensions;

	std::vector<VulkanPhysicalDevice> PhysicalDevices;

	uint32_t ApiVersion = {};
	VkInstance Instance = VK_NULL_HANDLE;

	bool DebugLayerActive = false;

	static void CheckVulkanError(VkResult result, const char* text)
	{
		if (result >= VK_SUCCESS) return;
		VulkanError((text + std::string(": ") + VkResultToString(result)).c_str());
	}

private:
	bool WantDebugLayer = false;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

	void CreateInstance();
	void ReleaseResources();

	static void InitVolk();
	static std::vector<VkLayerProperties> GetAvailableLayers();
	static std::vector<VkExtensionProperties> GetExtensions();
	static std::vector<VulkanPhysicalDevice> GetPhysicalDevices(VkInstance instance, uint32_t apiVersion);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	static std::vector<std::string> SplitString(const std::string& s, const std::string& seperator);
};
