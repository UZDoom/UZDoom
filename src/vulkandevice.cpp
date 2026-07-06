
#include "vulkandevice.h"
#include "vulkanobjects.h"
#include "vulkancompatibledevice.h"
#include <algorithm>
#include <set>
#include <string>

VulkanDevice::VulkanDevice(std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanSurface> surface, const VulkanCompatibleDevice& selectedDevice) : Instance(instance), Surface(surface)
{
	PhysicalDevice = *selectedDevice.Device;
	EnabledDeviceExtensions = selectedDevice.EnabledDeviceExtensions;
	EnabledFeatures = selectedDevice.EnabledFeatures;

	GraphicsFamily = selectedDevice.GraphicsFamily;
	PresentFamily = selectedDevice.PresentFamily;
	GraphicsTimeQueries = selectedDevice.GraphicsTimeQueries;

	try
	{
		CreateDevice();
		CreateAllocator();
	}
	catch (...)
	{
		ReleaseResources();
		throw;
	}
}

VulkanDevice::~VulkanDevice()
{
	ReleaseResources();
}

bool VulkanDevice::SupportsExtension(const char* ext) const
{
	return
		EnabledDeviceExtensions.find(ext) != EnabledDeviceExtensions.end() ||
		Instance->EnabledExtensions.find(ext) != Instance->EnabledExtensions.end();
}

#ifndef VK_KHR_MAINTENANCE4_EXTENSION_NAME
#define VK_KHR_MAINTENANCE4_EXTENSION_NAME "VK_KHR_maintenance4"
#endif

void VulkanDevice::CreateAllocator()
{
	VmaAllocatorCreateInfo allocinfo = {};
	allocinfo.vulkanApiVersion = Instance->ApiVersion;
	if (SupportsExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) && SupportsExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
		allocinfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	if (SupportsExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
		allocinfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	if (SupportsExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME))
		allocinfo.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
	//if (SupportsExtension(VK_KHR_MAINTENANCE4_EXTENSION_NAME))
	//	allocinfo.flags |= MA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
	if (SupportsExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
		allocinfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	if (SupportsExtension(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME))
		allocinfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
	if (SupportsExtension(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME))
		allocinfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
	allocinfo.physicalDevice = PhysicalDevice.Device;
	allocinfo.device = device;
	allocinfo.instance = Instance->Instance;
	allocinfo.preferredLargeHeapBlockSize = 64 * 1024 * 1024;
	if (vmaCreateAllocator(&allocinfo, &allocator) != VK_SUCCESS)
		VulkanError("Unable to create allocator");
}

void VulkanDevice::CreateDevice()
{
	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<int> neededFamilies;
	if (GraphicsFamily != -1)
		neededFamilies.insert(GraphicsFamily);
	if (PresentFamily != -1)
		neededFamilies.insert(PresentFamily);

	for (int index : neededFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCreateInfo.queueFamilyIndex = index;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	std::vector<const char*> extensionNames;
	extensionNames.reserve(EnabledDeviceExtensions.size());
	for (const auto& name : EnabledDeviceExtensions)
		extensionNames.push_back(name.c_str());

	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
	deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
	deviceCreateInfo.enabledLayerCount = 0;

	VkPhysicalDeviceFeatures2 deviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures2.features = EnabledFeatures.Features;

	void** next = const_cast<void**>(&deviceCreateInfo.pNext);
	if (SupportsExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		*next = &deviceFeatures2;
		next = &deviceFeatures2.pNext;
	}
	else // vulkan 1.0 specified features in a different way
	{
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures2.features;
	}

	if (SupportsExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
	{
		*next = &EnabledFeatures.BufferDeviceAddress;
		next = &EnabledFeatures.BufferDeviceAddress.pNext;
	}
	if (SupportsExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
	{
		*next = &EnabledFeatures.AccelerationStructure;
		next = &EnabledFeatures.AccelerationStructure.pNext;
	}
	if (SupportsExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME))
	{
		*next = &EnabledFeatures.RayQuery;
		next = &EnabledFeatures.RayQuery.pNext;
	}
	if (SupportsExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
	{
		*next = &EnabledFeatures.DescriptorIndexing;
		next = &EnabledFeatures.DescriptorIndexing.pNext;
	}
	if (SupportsExtension(VK_EXT_DEVICE_FAULT_EXTENSION_NAME))
	{
		*next = &EnabledFeatures.Fault;
		next = &EnabledFeatures.Fault.pNext;
	}
	if (SupportsExtension(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME))
	{
		*next = &EnabledFeatures.GraphicsPipelineLibrary;
		next = &EnabledFeatures.GraphicsPipelineLibrary.pNext;
	}

	VkResult result = vkCreateDevice(PhysicalDevice.Device, &deviceCreateInfo, nullptr, &device);
	CheckVulkanError(result, "Could not create vulkan device");

	volkLoadDevice(device);

	if (GraphicsFamily != -1)
		vkGetDeviceQueue(device, GraphicsFamily, 0, &GraphicsQueue);
	if (PresentFamily != -1)
		vkGetDeviceQueue(device, PresentFamily, 0, &PresentQueue);
}

void VulkanDevice::ReleaseResources()
{
	if (device)
		vkDeviceWaitIdle(device);

	if (allocator)
		vmaDestroyAllocator(allocator);

	if (device)
		vkDestroyDevice(device, nullptr);
	device = nullptr;
}

void VulkanDevice::SetObjectName(const char* name, uint64_t handle, VkObjectType type)
{
	if (!Instance->DebugLayerActive) return;

	VkDebugUtilsObjectNameInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	info.objectHandle = handle;
	info.objectType = type;
	info.pObjectName = name;
	vkSetDebugUtilsObjectNameEXT(device, &info);
}

VulkanDeviceFaultInfo VulkanDevice::GetDeviceFaultInfo()
{
	if (!SupportsExtension(VK_EXT_DEVICE_FAULT_EXTENSION_NAME) || !EnabledFeatures.Fault.deviceFault)
		return {};

	VkDeviceFaultCountsEXT counts = { VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT };
	VkResult result = vkGetDeviceFaultInfoEXT(device, &counts, nullptr);
	if (result != VK_INCOMPLETE && result != VK_SUCCESS)
		return {};

	std::vector<VkDeviceFaultAddressInfoEXT> addressInfos(counts.addressInfoCount);
	std::vector<VkDeviceFaultVendorInfoEXT> vendorInfos(counts.vendorInfoCount);
	std::vector<uint8_t> vendorBinaryData(counts.vendorBinarySize);

	VkDeviceFaultInfoEXT info = { VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT };
	info.pAddressInfos = addressInfos.data();
	info.pVendorInfos = vendorInfos.data();
	info.pVendorBinaryData = vendorBinaryData.data();

	result = vkGetDeviceFaultInfoEXT(device, &counts, &info);
	if (result != VK_SUCCESS)
		return {};

	VulkanDeviceFaultInfo lostinfo;
	lostinfo.description = info.description;

	/*
	for (const VkDeviceFaultAddressInfoEXT& addressInfo : addressInfos)
	{
		// To do: does vk_mem_alloc have anything that helps us map an address to an allocation?
	}
	*/

	for (const VkDeviceFaultVendorInfoEXT& vendorInfo : vendorInfos)
	{
		lostinfo.vendorInfos.push_back(vendorInfo.description);
	}

	return lostinfo;
}
