/*
*  VulkanDevice.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "VulkanDevice.hpp"
#include <EngineDefines.hpp>

using namespace Kompot;

VulkanDevice::VulkanDevice(const vk::Instance &vkInstance, const vk::PhysicalDevice& vkPhysicalDevice) :
        mVkInstance(vkInstance), mVkPhysicalDevice(vkPhysicalDevice)
{
    check(mVkInstance);
    check(mVkPhysicalDevice);

    vk::DeviceCreateInfo vkDeviceCreateInfo{};
    //vkDeviceCreateInfo.setPQueueCreateInfos()

    const auto createDeviceResult = mVkPhysicalDevice.createDevice(vkDeviceCreateInfo, nullptr, mVkDevice);
}

Kompot::VulkanDevice::~VulkanDevice()
{
    if (mVkPhysicalDevice)
    {

    }
    mVkDevice.destroy(nullptr);
    mVkDevice = nullptr;
}
