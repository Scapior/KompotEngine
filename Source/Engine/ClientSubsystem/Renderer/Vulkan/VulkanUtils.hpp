/*
*  VulkanUtils.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/


#pragma once

#include <EngineTypes.hpp>
#include <vulkan/vulkan.hpp>
#include <optional>

namespace Kompot::VulkanUtils
{
    // instance
    std::vector<const char*> getRequiredInstanceExtensions();

    // physical device selection
    struct DeviceComparsionAttributes
    {
        uint64_t memorySize;
        bool isDiscreteDevice;
    };
    DeviceComparsionAttributes getDeviceComparsionAttributes(const vk::PhysicalDevice& vkPhysicalDevice, const vk::MemoryPropertyFlagBits memoryFlags);

    // logical device selection
    std::vector<const char*> getRequiredExtensions();
    std::vector<const char*> getRequiredValidationLayers();

    struct QueueFamilies
    {
        std::optional<uint32_t> graphicsIndex;
        std::uint32_t graphicsCount;

        std::optional<uint32_t> computeIndex;
        std::uint32_t computeCount;

        std::optional<uint32_t> transferIndex;
        std::uint32_t transferCount;

        bool hasAllIndicies() const;
    };
    QueueFamilies selectQueuesFamilies(const vk::PhysicalDevice& vkPhysicalDevice);

}
