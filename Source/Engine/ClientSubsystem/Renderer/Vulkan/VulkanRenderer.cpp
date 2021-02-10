/*
 *  VulkanRenderer.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "VulkanRenderer.hpp"
#include <Engine/ClientSubsystem/Window/Window.hpp>
#include <Engine/ErrorHandling.hpp>
#include <Engine/Log/Log.hpp>
#include <EngineDefines.hpp>
#include <vector>

using namespace Kompot;

VulkanRenderer::VulkanRenderer() : mVkInstance(nullptr)
{
    createInstance();
    setupDebugCallback();
    mVulkanDevice.reset(new VulkanDevice(mVkInstance, selectPhysicalDevice()));
};

VulkanRenderer::~VulkanRenderer()
{
    mVulkanDevice.release();
    deleteDebugCallback();
    mVkInstance.destroy(nullptr);
}

void VulkanRenderer::createInstance()
{
    vk::ApplicationInfo vkApplicationInfo{ENGINE_NAME, 0, ENGINE_NAME, 0, VK_MAKE_VERSION(1, 2, 0)};

    const auto instanceExtensions = VulkanUtils::getRequiredInstanceExtensions();
    auto vkInstanceCreateInfo     = vk::InstanceCreateInfo{}.setPApplicationInfo(&vkApplicationInfo).setPEnabledExtensionNames(instanceExtensions);

    if (const auto result = vk::createInstance(vkInstanceCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkInstance = result.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create vkCreateInstance, result code \"" + vk::to_string(result.result) + "\"");
    }
}

vk::PhysicalDevice VulkanRenderer::selectPhysicalDevice()
{
    uint32_t physicalDevicesCount = 0;
    if (const auto enumerateCountResult = mVkInstance.enumeratePhysicalDevices(&physicalDevicesCount, nullptr);
        enumerateCountResult != vk::Result::eSuccess)
    {
        Kompot::ErrorHandling::exit("Failed to get physical devices count, result code \"" + vk::to_string(enumerateCountResult) + "\"");
    }

    std::vector<vk::PhysicalDevice> physicalDevices(physicalDevicesCount);
    if (const auto enumerateCountResult = mVkInstance.enumeratePhysicalDevices(&physicalDevicesCount, physicalDevices.data());
        enumerateCountResult != vk::Result::eSuccess)
    {
        Kompot::ErrorHandling::exit("Failed to enumerate physical devices, result code \"" + vk::to_string(enumerateCountResult) + "\"");
    }

    if (physicalDevices.empty())
    {
        Kompot::ErrorHandling::exit("\"No one GPU has founded\"");
    }

    uint32_t selectedDeviceIndex = 0;
    VulkanUtils::DeviceComparsionAttributes selectedDeviceAttributes{};
    bool hasDiscreteDeviceWasFound = false;
    for (auto i = 0u; i < physicalDevices.size(); ++i)
    {
        const auto& physicalDevice   = physicalDevices[i];
        const auto& deviceAttributes = VulkanUtils::getDeviceComparsionAttributes(physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);

        const bool isHaveMoreMemory             = deviceAttributes.memorySize > selectedDeviceAttributes.memorySize;
        const bool isMoreBetterDevice           = (deviceAttributes.isDiscreteDevice == hasDiscreteDeviceWasFound) && isHaveMoreMemory;
        const bool isFirstFoundedDiscreteDevice = deviceAttributes.isDiscreteDevice && !hasDiscreteDeviceWasFound;

        const bool isNeedToSelectThisDevice = isMoreBetterDevice || isFirstFoundedDiscreteDevice;

        if (isNeedToSelectThisDevice)
        {
            hasDiscreteDeviceWasFound = hasDiscreteDeviceWasFound | deviceAttributes.isDiscreteDevice;
            selectedDeviceIndex       = i;
            selectedDeviceAttributes  = deviceAttributes;
        }
    }

    return physicalDevices[selectedDeviceIndex];
}
WindowRendererAttributes* VulkanRenderer::updateWindowAttributes(Window* window)
{
    if (!window)
    {
        return nullptr;
    }
    mWindows.emplace(window);

    check(mVulkanDevice->logicDevice().waitIdle() == vk::Result::eSuccess);

    VulkanWindowRendererAttributes* result;
    auto windowAttributes = window->getWindowRendererAttributes();
    if (auto vulkanWindowAttributes = dynamic_cast<VulkanWindowRendererAttributes*>(windowAttributes))
    {
        result = vulkanWindowAttributes;
    }
    else
    {
        if (windowAttributes)
        {
            delete windowAttributes;
        }
        result = new VulkanWindowRendererAttributes;
    }
    cleanupWindowSwapchain(result);
    result->scissor.extent = vk::Extent2D{window->getWidth(), window->getHeight()};
    if (result && !result->surface)
    {
        result->surface = window->createVulkanSurface();
    }
    recreateSwapchain(result);

    //    createImageViews();
    //    createRenderPass();
    //    createGraphicsPipeline();
    //    createFramebuffers();
    //    createCommandBuffers();

    return result;
}

void VulkanRenderer::unregisterWindow(Window* window)
{
    if (!window)
    {
        return;
    }
    auto windowAttributes = window->getWindowRendererAttributes();
    if (auto vulkanWindowAttributes = dynamic_cast<VulkanWindowRendererAttributes*>(windowAttributes))
    {
        cleanupWindowSwapchain(vulkanWindowAttributes);
        mVkInstance.destroySurfaceKHR(vulkanWindowAttributes->surface);
    }

    mWindows.erase(window);
}

void VulkanRenderer::recreateSwapchain(VulkanWindowRendererAttributes* windowAttributes)
{
    if (!windowAttributes)
    {
        return;
    }

    if (auto surfaceCapabilitiesResult = mVulkanDevice->physicalDevice().getSurfaceCapabilitiesKHR(windowAttributes->surface);
        surfaceCapabilitiesResult.result == vk::Result::eSuccess)
    {
        auto& vkSurfaceCapabilities = surfaceCapabilitiesResult.value;

        const auto foundPresentQueue = mVulkanDevice->findPresentQueue(windowAttributes);
        const auto graphicQueueIndex = mVulkanDevice->getGraphicsQueueIndex();

        const std::array<uint32_t, 2> queueFamilyIndices = {foundPresentQueue.second, graphicQueueIndex};
        const bool bQueuesAreSame                        = graphicQueueIndex == foundPresentQueue.second;

        const auto swapchainFormat = vk::Format::eB8G8R8A8Srgb; // ToDo: add selection based on GPU capabilities
        // VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR  ?
        auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
                                       .setSurface(windowAttributes->surface)
                                       .setMinImageCount(vkSurfaceCapabilities.maxImageCount)
                                       .setImageFormat(swapchainFormat)
                                       .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
                                       .setImageExtent(windowAttributes->scissor.extent)
                                       .setImageArrayLayers(1)
                                       .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                                       .setImageSharingMode(bQueuesAreSame ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent)
                                       .setQueueFamilyIndices(queueFamilyIndices)
                                       .setPreTransform(vkSurfaceCapabilities.currentTransform)
                                       .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                                       .setPresentMode(vk::PresentModeKHR::eMailbox)
                                       .setClipped(VK_TRUE)
                                       .setOldSwapchain(windowAttributes->swapchain.handler);

        const auto& logicalDevice = mVulkanDevice->logicDevice();
        auto& swapchain = windowAttributes->swapchain;
        if (const auto result = logicalDevice.createSwapchainKHR(swapchainCreateInfo); result.result == vk::Result::eSuccess)
        {
            swapchain.handler = result.value;
            windowAttributes->swapchain.format  = swapchainFormat;
        }
        if (const auto result = logicalDevice.getSwapchainImagesKHR(swapchain.handler); result.result == vk::Result::eSuccess)
        {
            swapchain.images = result.value;
        }
        swapchain.imageViews.resize(swapchain.images.size());
        for (std::size_t i = 0; i < swapchain.imageViews.size(); ++i)
        {
            const auto imageViewCreateInfo = vk::ImageViewCreateInfo{}
            .setImage(swapchain.images[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(swapchain.format)
            .setComponents(vk::ComponentMapping{})
            .setSubresourceRange(vk::ImageSubresourceRange{}.setAspectMask(vk::ImageAspectFlagBits::eColor).setLevelCount(1).setLayerCount(1));
            if (const auto result = logicalDevice.createImageView(imageViewCreateInfo); result.result == vk::Result::eSuccess)
            {
                swapchain.imageViews[i] = result.value;
            }
            else
            {
                Kompot::ErrorHandling::exit("Failed to create an image view for swapchain image, result code \"" + vk::to_string(result.result) + "\"");
            }
        }
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    return vulkanDebugCallbackImpl(
        vk::DebugUtilsMessageSeverityFlagsEXT(messageSeverity),
        vk::DebugUtilsMessageTypeFlagsEXT(messageType),
        vk::DebugUtilsMessengerCallbackDataEXT(*callbackData),
        userData);
}

VkBool32 VulkanRenderer::vulkanDebugCallbackImpl(
    vk::DebugUtilsMessageSeverityFlagsEXT messageSeverite,
    vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT& callbackData,
    void* userData)
{
    Log::getInstance() << Log::DateTimeBlock << "[Validation layer " << vk::to_string(messageType) << vk::to_string(messageSeverite) << "] "
                       << callbackData.pMessage << std::endl;
    return VK_SUCCESS;
}

void VulkanRenderer::setupDebugCallback()
{
#ifdef ENGINE_DEBUG
    auto createDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(mVkInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (!mVkInstance || !createDebugUtilsMessengerEXT)
    {
        return;
    }

    using SaverityFlagBits = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using TypeFlagBits     = vk::DebugUtilsMessageTypeFlagBitsEXT;
    const VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo =
        vk::DebugUtilsMessengerCreateInfoEXT{}
            .setMessageSeverity(SaverityFlagBits::eError | SaverityFlagBits::eInfo | SaverityFlagBits::eVerbose | SaverityFlagBits::eWarning)
            .setMessageType(TypeFlagBits::ePerformance | TypeFlagBits::eValidation)
            .setPfnUserCallback(&VulkanRenderer::vulkanDebugCallback);

    VkDebugUtilsMessengerEXT DebugUtilsMessengerHandler{};
    if (vk::Result result{createDebugUtilsMessengerEXT(mVkInstance, &debugUtilsMessengerCreateInfo, nullptr, &DebugUtilsMessengerHandler)};
        result == vk::Result::eSuccess)
    {
        mVkDebugUtilsMessenger = DebugUtilsMessengerHandler;
    }
    else
    {
        Log::getInstance() << "Failed to createDebugUtilsMessengerEXT, result code \"" << vk::to_string(result) << "\"" << std::endl;
    }
#endif
}

void VulkanRenderer::deleteDebugCallback()
{
#ifdef ENGINE_DEBUG
    auto destroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(mVkInstance, "vkDestroyDebugUtilsMessengerEXT"));
    if (destroyDebugUtilsMessengerEXT && mVkInstance && mVkDebugUtilsMessenger)
    {
        destroyDebugUtilsMessengerEXT(mVkInstance, mVkDebugUtilsMessenger, nullptr);
    }
#endif
}
void VulkanRenderer::cleanupWindowSwapchain(VulkanWindowRendererAttributes* windowAttributes)
{
    auto& logicDevice = mVulkanDevice->logicDevice();
    auto& swapchain = windowAttributes->swapchain;
    for (auto& imageView : swapchain.imageViews)
    {
        logicDevice.destroy(imageView);
    }
    swapchain.images.resize(0);
    swapchain.imageViews.resize(0);
    if (windowAttributes->swapchain.handler)
    {
        logicDevice.destroy(swapchain.handler);
    }
    //    for (auto framebuffer : swapChainFramebuffers) {
    //        vkDestroyFramebuffer(device, framebuffer, nullptr);
    //    }
    //
    //    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    //
    //    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    //    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    //    vkDestroyRenderPass(device, renderPass, nullptr);
    //
    //    for (auto imageView : swapChainImageViews) {
    //        vkDestroyImageView(device, imageView, nullptr);
    //    }
    //
    //    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
