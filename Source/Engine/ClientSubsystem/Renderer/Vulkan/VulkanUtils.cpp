/*
*  VulkanUtils.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "VulkanUtils.hpp"
#include <unordered_set>

using namespace Kompot;

VulkanUtils::DeviceComparsionAttributes VulkanUtils::getDeviceComparsionAttributes(const vk::PhysicalDevice& vkPhysicalDevice, const vk::MemoryPropertyFlagBits memoryFlags)
{
    DeviceComparsionAttributes deviceComparsionAttributes{};
    uint64_t deviceMemoryCount = 0;

    const vk::PhysicalDeviceProperties2 physicalDeviceProperties = vkPhysicalDevice.getProperties2();
    deviceComparsionAttributes.isDiscreteDevice = physicalDeviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;

    const vk::PhysicalDeviceMemoryProperties2 physicalDeviceMemoryProperties = vkPhysicalDevice.getMemoryProperties2();

    std::unordered_set<uint32_t> suitableHeapsIndices;
    for (auto i = 0u; i < physicalDeviceMemoryProperties.memoryProperties.memoryTypeCount; ++i)
    {
        const auto& memoryType = physicalDeviceMemoryProperties.memoryProperties.memoryTypes[i];
        if (memoryType.propertyFlags & memoryFlags)
        {
            suitableHeapsIndices.emplace(memoryType.heapIndex);
        }
    }

    for (const auto& heapIndex : suitableHeapsIndices)
    {
        deviceComparsionAttributes.memorySize += physicalDeviceMemoryProperties.memoryProperties.memoryHeaps[heapIndex].size;
    }

    return deviceComparsionAttributes;
}
