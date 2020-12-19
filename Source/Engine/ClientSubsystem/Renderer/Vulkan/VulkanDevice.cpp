/*
*  VulkanDevice.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "VulkanDevice.hpp"
#include "VulkanUtils.hpp"
#include <EngineDefines.hpp>
#include <Engine/Log/Log.hpp>

using namespace Kompot;

VulkanDevice::VulkanDevice(const vk::Instance &vkInstance, const vk::PhysicalDevice& vkPhysicalDevice) :
        mVkInstance(vkInstance), mVkPhysicalDevice(vkPhysicalDevice)
{
    check(mVkInstance);
    check(mVkPhysicalDevice);

    const auto selectedQueueFamilies = VulkanUtils::selectQueuesFamilies(mVkPhysicalDevice);
    if (!selectedQueueFamilies.hasAllIndicies())
    {
        Log::getInstance() << "Not all queue families was found" << std::endl;
        std::exit(-1);
    }

    std::array<vk::DeviceQueueCreateInfo, 3> queuesCreateInfos{};

    queuesCreateInfos[0].queueFamilyIndex = selectedQueueFamilies.graphicsIndex.value();
    queuesCreateInfos[0].queueCount = selectedQueueFamilies.graphicsCount;
    std::vector<float> graphicsQueuePriorities (selectedQueueFamilies.graphicsCount);
    queuesCreateInfos[0].pQueuePriorities = graphicsQueuePriorities.data();

    queuesCreateInfos[1].queueFamilyIndex = selectedQueueFamilies.computeIndex.value();
    queuesCreateInfos[1].queueCount = selectedQueueFamilies.computeCount;
    std::vector<float> computeQueuePriorities (selectedQueueFamilies.computeCount);
    queuesCreateInfos[0].pQueuePriorities = computeQueuePriorities.data();

    queuesCreateInfos[2].queueFamilyIndex = selectedQueueFamilies.transferIndex.value();
    queuesCreateInfos[2].queueCount = selectedQueueFamilies.transferCount;
    std::vector<float> transferQueuePriorities (selectedQueueFamilies.transferCount);
    queuesCreateInfos[0].pQueuePriorities = transferQueuePriorities.data();

    const auto extensions = VulkanUtils::getRequiredExtensions();
    const auto validationLayers = VulkanUtils::getRequiredValidationLayers();

    auto vkDeviceCreateInfo = vk::DeviceCreateInfo()
        .setQueueCreateInfos(queuesCreateInfos)
        .setPEnabledExtensionNames(extensions)
        .setPEnabledLayerNames(validationLayers);

    if (const auto result = mVkPhysicalDevice.createDevice(vkDeviceCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkDevice = result.value;
    }
    else
    {
        Log::getInstance() << "Failed to create vkDevice, result code \""<<  vk::to_string(result.result) << "\"" << std::endl;
        std::exit(-1);
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
