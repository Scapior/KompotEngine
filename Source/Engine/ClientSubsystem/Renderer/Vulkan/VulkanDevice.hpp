/*
 *  VulkanDevice.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include "VulkanUtils.hpp"
#include "VulkanTypes.hpp"
#include <vulkan/vulkan.hpp>
#include <optional>
#include <utility>


namespace Kompot::Rendering::Vulkan
{
class VulkanDevice
{
public:
    VulkanDevice(const vk::Instance& vkInstance, const vk::PhysicalDevice& vkPhysicalDevice);
    ~VulkanDevice();

    const vk::Device asLogicDevice() const
    {
        return mVkDevice;
    }

    const vk::PhysicalDevice asPhysicalDevice() const
    {
        return mVkPhysicalDevice;
    }

    const vk::Queue getGraphicsQueue() const
    {
        return mGraphicsQueue;
    }

    const vk::Queue getTransferQueue() const
    {
        return mTransferQueue;
    }

    const vk::Queue getComputeQueue() const
    {
        return mComputeQueue;
    }

    uint32_t getGraphicsQueueIndex() const
    {
        return queueFamilies.graphicsIndex.value();
    }

    uint32_t getTransferQueueIndex() const
    {
        return queueFamilies.transferIndex.value();
    }

    uint32_t getComputeQueueIndex() const
    {
        return queueFamilies.computeIndex.value();
    }

    std::pair<vk::Queue, uint32_t> findPresentQueue(const VulkanWindowRendererAttributes* windowAttributes) const;

private:
    vk::Instance mVkInstance;

    vk::PhysicalDevice mVkPhysicalDevice;
    vk::Device mVkDevice;

    Kompot::Rendering::Vulkan::Utils::QueueFamilies queueFamilies;

    vk::Queue mGraphicsQueue;
    vk::Queue mTransferQueue;
    vk::Queue mComputeQueue;
};

} // namespace Kompot
