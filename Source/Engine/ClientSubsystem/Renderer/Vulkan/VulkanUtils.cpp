/*
 *  VulkanUtils.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "VulkanUtils.hpp"
#include <EngineDefines.hpp>
#include <unordered_set>

using namespace Kompot;

VulkanUtils::DeviceComparsionAttributes VulkanUtils::getDeviceComparsionAttributes(
    const vk::PhysicalDevice& vkPhysicalDevice,
    const vk::MemoryPropertyFlagBits memoryFlags)
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

std::vector<const char*> VulkanUtils::getRequiredExtensions()
{
    return {}; // ToDo
}

std::vector<const char*> VulkanUtils::getRequiredValidationLayers()
{
#ifdef ENGINE_DEBUG
    return {}; // ToDo
#else
    return {};
#endif
}

VulkanUtils::QueueFamilies VulkanUtils::selectQueuesFamilies(const vk::PhysicalDevice& vkPhysicalDevice)
{
    QueueFamilies result;
    const auto queueFamilyProperties = vkPhysicalDevice.getQueueFamilyProperties();
    for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        const auto& queueFamily = queueFamilyProperties[i];

        const bool isHaveGraphicsFlag{queueFamily.queueFlags & vk::QueueFlagBits::eGraphics};
        const bool isHaveComputeFlag{queueFamily.queueFlags & vk::QueueFlagBits::eCompute};
        const bool isHaveTransferFlag{queueFamily.queueFlags & vk::QueueFlagBits::eTransfer};

        if (isHaveGraphicsFlag && !result.graphicsIndex.has_value())
        {
            result.graphicsIndex = i;
            result.graphicsCount = queueFamily.queueCount;
        }

        if (isHaveComputeFlag && (!result.computeIndex.has_value() || result.graphicsIndex.value() != i))
        {
            result.computeIndex = i;
            result.computeCount = queueFamily.queueCount;
        }

        if (isHaveTransferFlag && !result.transferIndex.has_value())
        {
            result.transferIndex = i;
            result.transferCount = queueFamily.queueCount;
        }

        if (result.hasAllIndicies())
        {
            break;
        }
    }
    return result;
}

std::vector<const char*> VulkanUtils::getRequiredInstanceExtensions()
{
    return {VK_KHR_SURFACE_EXTENSION_NAME};
}

bool VulkanUtils::QueueFamilies::hasAllIndicies() const
{
    return graphicsIndex.has_value() && computeIndex.has_value() && transferIndex.has_value();
}