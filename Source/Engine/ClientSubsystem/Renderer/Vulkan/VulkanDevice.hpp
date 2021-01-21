/*
 *  VulkanDevice.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include <vulkan/vulkan.hpp>

namespace Kompot
{
class VulkanDevice
{
public:
    VulkanDevice(const vk::Instance& vkInstance, const vk::PhysicalDevice& vkPhysicalDevice);
    ~VulkanDevice();

private:
    vk::Instance mVkInstance;

    vk::PhysicalDevice mVkPhysicalDevice;
    vk::Device mVkDevice;
};

} // namespace Kompot
