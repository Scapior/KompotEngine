/*
*  VulkanRenderer.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "VulkanRenderer.hpp"
#include "VulkanUtils.hpp"
#include <Engine/Log/Log.hpp>
#include <EngineDefines.hpp>

using namespace Kompot;

VulkanRenderer::VulkanRenderer() :
    mVkInstance(nullptr)
{
    createInstance();
    mVulkanDevice.reset(new VulkanDevice(mVkInstance, selectPhysicalDevice()));
}

VulkanRenderer::~VulkanRenderer()
{
    mVulkanDevice.release();
    vkDestroyInstance(mVkInstance, nullptr);
    mVkInstance.destroy(nullptr);
    mVkInstance = nullptr;
}

void VulkanRenderer::createInstance()
{
    vk::ApplicationInfo vkApplicationInfo{ENGINE_NAME, 0, ENGINE_NAME, 0, VK_MAKE_VERSION(1,2,0) };

    const auto instanceExtensions = VulkanUtils::getRequiredInstanceExtensions();
    auto vkInstanceCreateInfo = vk::InstanceCreateInfo{}
        .setPApplicationInfo(&vkApplicationInfo)
        .setPEnabledExtensionNames(instanceExtensions);

    if (const auto result = vk::createInstance(vkInstanceCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkInstance = result.value;
    }
    else
    {
         Log::getInstance() << "Failed to create vkCreateInstance, result code \""<<  vk::to_string(result.result) << "\"" << std::endl;
         std::exit(-1);
    }
}

vk::PhysicalDevice VulkanRenderer::selectPhysicalDevice()
{
    uint32_t physicalDevicesCount = 0;
    if (const auto enumerateCountResult = mVkInstance.enumeratePhysicalDevices(&physicalDevicesCount, nullptr); enumerateCountResult != vk::Result::eSuccess)
    {
         Log::getInstance() << "Failed to get physical devices count, result code \"" <<  vk::to_string(enumerateCountResult) << "\"" << std::endl;
         std::exit(-1);
    }

    std::vector<vk::PhysicalDevice> physicalDevices(physicalDevicesCount);
    if (const auto enumerateCountResult = mVkInstance.enumeratePhysicalDevices(&physicalDevicesCount, physicalDevices.data()); enumerateCountResult != vk::Result::eSuccess)
    {
        Log::getInstance() << "Failed to enumerate physical devices, result code \"" <<  vk::to_string(enumerateCountResult) << "\"" << std::endl;
        std::exit(-1);
    }

    if (physicalDevices.empty())
    {
        Log::getInstance() << "No one GPU has founded" << std::endl;
        std::exit(-1);
    }

    uint32_t selectedDeviceIndex = 0;
    VulkanUtils::DeviceComparsionAttributes selectedDeviceAttributes;
    bool hasDiscreteDeviceWasFound = false;
    for (auto i = 0u; i < physicalDevices.size(); ++i)
    {
        const auto& physicalDevice = physicalDevices[i];
        const auto& deviceAttributes = VulkanUtils::getDeviceComparsionAttributes(physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);

        const bool isHaveMoreMemory = deviceAttributes.memorySize > selectedDeviceAttributes.memorySize;
        const bool isMoreBetterDevice = (deviceAttributes.isDiscreteDevice == hasDiscreteDeviceWasFound) && isHaveMoreMemory;
        const bool isFirstFoundedDiscreteDevice = deviceAttributes.isDiscreteDevice && !hasDiscreteDeviceWasFound;

        const bool isNeedToSelectThisDevice = isMoreBetterDevice || isFirstFoundedDiscreteDevice;

        if (isNeedToSelectThisDevice)
        {
            hasDiscreteDeviceWasFound = hasDiscreteDeviceWasFound | deviceAttributes.isDiscreteDevice;
            selectedDeviceIndex = i;
            selectedDeviceAttributes = deviceAttributes;
        }
    }

    return physicalDevices[selectedDeviceIndex];
}
