/*
 *  VulkanDevice.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "VulkanDevice.hpp"
#include <Engine/ErrorHandling.hpp>
#include <Engine/Log/Log.hpp>
#include <EngineDefines.hpp>
#include <set>

using namespace Kompot;

VulkanDevice::VulkanDevice(const vk::Instance& vkInstance, const vk::PhysicalDevice& vkPhysicalDevice) :
    mVkInstance(vkInstance), mVkPhysicalDevice(vkPhysicalDevice)
{
    check(mVkInstance);
    check(mVkPhysicalDevice);

    queueFamilies = VulkanUtils::selectQueuesFamilies(mVkPhysicalDevice);
    if (!queueFamilies.hasAllIndicies())
    {
        Kompot::ErrorHandling::exit("Not all queue families was found");
    }

    const std::set<std::uint32_t> uniqueQueueFamilyIndicies = queueFamilies.getUniqueQueueFamilyIndicies();

    std::vector<vk::DeviceQueueCreateInfo> queuesCreateInfos{uniqueQueueFamilyIndicies.size()};
    std::vector<std::vector<float>> queuePriorities{uniqueQueueFamilyIndicies.size()};

    auto uniqueQueueFamilyIndiciesIterator = uniqueQueueFamilyIndicies.begin();

    for (std::size_t i = 0; i < uniqueQueueFamilyIndicies.size(); ++i)
    {
        queuePriorities[i].resize(queueFamilies.computeCount);
        queuesCreateInfos[i]
            .setQueueFamilyIndex(*uniqueQueueFamilyIndiciesIterator++)
            .setQueueCount(queueFamilies.graphicsCount)
            .setQueuePriorities(queuePriorities[i]);
    }

    const auto extensions       = VulkanUtils::getRequiredDeviceExtensions();
    const auto validationLayers = VulkanUtils::getRequiredDeviceValidationLayers();

    auto vkDeviceCreateInfo =
        vk::DeviceCreateInfo().setQueueCreateInfos(queuesCreateInfos).setPEnabledExtensionNames(extensions).setPEnabledLayerNames(validationLayers);

    if (const auto result = mVkPhysicalDevice.createDevice(vkDeviceCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkDevice = result.value;
        // ToDo: only one queue? maybe we need a queue manager
        mGraphicsQueue = mVkDevice.getQueue(queueFamilies.graphicsIndex.value(), 0);
        mComputeQueue  = mVkDevice.getQueue(queueFamilies.computeIndex.value(), 0);
        mTransferQueue = mVkDevice.getQueue(queueFamilies.transferIndex.value(), 0);
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create vkDevice, result code \"" + vk::to_string(result.result) + "\"");
    }
}

Kompot::VulkanDevice::~VulkanDevice()
{
    if (mVkPhysicalDevice)
    {
    }
    mVkDevice.destroy(nullptr);
    mVkDevice = nullptr;
}

std::pair<vk::Queue, uint32_t> VulkanDevice::findPresentQueue(const VulkanWindowRendererAttributes* windowAttributes) const
{
    std::pair<vk::Queue, uint32_t> foundQueue = {};
    if (windowAttributes)
    {
        std::optional<uint32_t> presentQueueFamilyIndex;

        const auto queueFamilyProperties = mVkPhysicalDevice.getQueueFamilyProperties();
        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            const auto& queueFamily    = queueFamilyProperties[i];
            uint32_t currentQueueIndex = static_cast<uint32_t>(i);
            if (auto result = mVkPhysicalDevice.getSurfaceSupportKHR(currentQueueIndex, windowAttributes->surface);
                result.result == vk::Result::eSuccess)
            {
                if (result.value == VK_TRUE)
                {
                    presentQueueFamilyIndex = currentQueueIndex;
                    break;
                }
            }
        }

        if (presentQueueFamilyIndex)
        {
            foundQueue = {mVkDevice.getQueue(presentQueueFamilyIndex.value(), 0), presentQueueFamilyIndex.value()};
        }
    }
    return foundQueue;
}
