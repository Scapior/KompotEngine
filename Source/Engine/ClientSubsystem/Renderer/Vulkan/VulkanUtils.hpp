/*
*  VulkanUtils.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/


#pragma once

#include <EngineTypes.hpp>
#include <vulkan/vulkan.hpp>

namespace Kompot::VulkanUtils
{
    struct DeviceComparsionAttributes
    {
        uint64_t memorySize;
        bool isDiscreteDevice;
    };

    DeviceComparsionAttributes getDeviceComparsionAttributes(const vk::PhysicalDevice& vkPhysicalDevice, const vk::MemoryPropertyFlagBits memoryFlags);

}
