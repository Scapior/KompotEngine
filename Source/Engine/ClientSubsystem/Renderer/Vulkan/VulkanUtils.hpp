/*
 *  VulkanUtils.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include <EngineTypes.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <set>

namespace Kompot::Rendering::Vulkan::Utils
{
// instance
std::vector<const char*> getRequiredInstanceExtensions();
std::vector<const char*> getRequiredInstanceValidationLayers();

// physical device selection
struct DeviceComparsionAttributes
{
    uint64_t memorySize;
    bool isDiscreteDevice;
};
DeviceComparsionAttributes getDeviceComparsionAttributes(const vk::PhysicalDevice& vkPhysicalDevice, const vk::MemoryPropertyFlagBits memoryFlags);

// logical device selection
std::vector<const char*> getRequiredDeviceExtensions();
std::vector<const char*> getRequiredDeviceValidationLayers();

struct QueueFamilies
{
    std::optional<std::uint32_t> graphicsIndex;
    std::uint32_t graphicsCount;

    std::optional<std::uint32_t> computeIndex;
    std::uint32_t computeCount;

    std::optional<std::uint32_t> transferIndex;
    std::uint32_t transferCount;

    bool hasAllIndicies() const;

    const std::set<std::uint32_t> getUniqueQueueFamilyIndicies();
};

QueueFamilies selectQueuesFamilies(const vk::PhysicalDevice& vkPhysicalDevice);

} // namespace Kompot::VulkanUtils
