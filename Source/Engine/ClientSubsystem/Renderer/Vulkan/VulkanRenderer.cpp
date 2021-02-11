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

    createCommands();
    createRenderpass();
    createSyncStructure();
};

VulkanRenderer::~VulkanRenderer()
{
    mVulkanDevice->logicDevice().destroy(mVkRenderPass);
    mVulkanDevice->logicDevice().destroy(mVkCommandPool);
    mMainCommandBuffer = nullptr;
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

    VulkanWindowRendererAttributes* windowAttributes;
    auto abstractWindowAttributes = window->getWindowRendererAttributes();
    if (auto vulkanWindowAttributes = dynamic_cast<VulkanWindowRendererAttributes*>(abstractWindowAttributes))
    {
        windowAttributes = vulkanWindowAttributes;
    }
    else
    {
        if (abstractWindowAttributes)
        {
            delete abstractWindowAttributes;
        }
        windowAttributes = new VulkanWindowRendererAttributes;
    }
    cleanupWindowSwapchain(windowAttributes);
    windowAttributes->scissor.extent = vk::Extent2D{window->getWidth(), window->getHeight()};
    if (windowAttributes && !windowAttributes->surface)
    {
        windowAttributes->surface = window->createVulkanSurface();
    }
    recreateWindowHandlers(windowAttributes);

    //    createImageViews();
    //    createRenderPass();
    //    createGraphicsPipeline();
    //    createFramebuffers();
    //    createCommandBuffers();

    return windowAttributes;
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

void VulkanRenderer::recreateWindowHandlers(VulkanWindowRendererAttributes* windowAttributes)
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

        mVkSwapchainFormat = vk::Format::eB8G8R8A8Srgb; // ToDo: add selection based on GPU capabilities
        // VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR  ?
        auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
                                       .setSurface(windowAttributes->surface)
                                       .setMinImageCount(vkSurfaceCapabilities.maxImageCount)
                                       .setImageFormat(mVkSwapchainFormat)
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
        auto& swapchain           = windowAttributes->swapchain;
        if (const auto result = logicalDevice.createSwapchainKHR(swapchainCreateInfo); result.result == vk::Result::eSuccess)
        {
            swapchain.handler = result.value;
        }
        if (const auto result = logicalDevice.getSwapchainImagesKHR(swapchain.handler); result.result == vk::Result::eSuccess)
        {
            swapchain.images = result.value;
        }
        swapchain.imageViews.resize(swapchain.images.size());

        swapchain.framebuffers.resize(swapchain.images.size());
        auto framebufferCreateInfo = vk::FramebufferCreateInfo{}
                                         .setRenderPass(mVkRenderPass)
                                         .setHeight(windowAttributes->scissor.extent.height)
                                         .setWidth(windowAttributes->scissor.extent.width)
                                         .setLayers(1)
                                         .setAttachmentCount(1);

        for (std::size_t i = 0; i < swapchain.imageViews.size(); ++i)
        {
            const auto imageViewCreateInfo =
                vk::ImageViewCreateInfo{}
                    .setImage(swapchain.images[i])
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(mVkSwapchainFormat)
                    .setComponents(vk::ComponentMapping{})
                    .setSubresourceRange(
                        vk::ImageSubresourceRange{}.setAspectMask(vk::ImageAspectFlagBits::eColor).setLevelCount(1).setLayerCount(1));
            if (const auto result = logicalDevice.createImageView(imageViewCreateInfo); result.result == vk::Result::eSuccess)
            {
                swapchain.imageViews[i] = result.value;
            }
            else
            {
                Kompot::ErrorHandling::exit(
                    "Failed to create an image view for swapchain image, result code \"" + vk::to_string(result.result) + "\"");
            }

            framebufferCreateInfo.setPAttachments(&swapchain.imageViews[i]);
            if (const auto result = logicalDevice.createFramebuffer(framebufferCreateInfo); result.result == vk::Result::eSuccess)
            {
                swapchain.framebuffers[i] = result.value;
            }
            else
            {
                Kompot::ErrorHandling::exit(
                    "Failed to create an framebuffer for swapchain image, result code \"" + vk::to_string(result.result) + "\"");
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
    auto& swapchain   = windowAttributes->swapchain;
    for (auto& framebuffer : swapchain.framebuffers)
    {
        logicDevice.destroy(framebuffer);
    }
    for (auto& imageView : swapchain.imageViews)
    {
        logicDevice.destroy(imageView);
    }
    swapchain.framebuffers.resize(0);
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

void VulkanRenderer::createCommands()
{
    const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo{}
                                           .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                           .setQueueFamilyIndex(mVulkanDevice->getGraphicsQueueIndex());

    if (const auto result = mVulkanDevice->logicDevice().createCommandPool(commandPoolCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkCommandPool = result.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create an CommandPool, result code \"" + vk::to_string(result.result) + "\"");
    }

    const auto commandBufferAllocateInfo =
        vk::CommandBufferAllocateInfo{}.setCommandPool(mVkCommandPool).setCommandBufferCount(1).setLevel(vk::CommandBufferLevel::ePrimary);

    if (const auto result = mVulkanDevice->logicDevice().allocateCommandBuffers(commandBufferAllocateInfo);
        result.result == vk::Result::eSuccess && result.value.size() == 1)
    {
        mMainCommandBuffer = result.value[0];
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create an CommandBuffer, result code \"" + vk::to_string(result.result) + "\"");
    }
}

void VulkanRenderer::createRenderpass()
{
    auto renderpassAttachmentDescription = vk::AttachmentDescription{}
                                               .setFormat(mVkSwapchainFormat)
                                               .setSamples(vk::SampleCountFlagBits::e1)
                                               .setLoadOp(vk::AttachmentLoadOp::eClear)
                                               .setStoreOp(vk::AttachmentStoreOp::eStore)
                                               .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                                               .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                               .setInitialLayout(vk::ImageLayout::eUndefined)
                                               .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    std::array renderpassAttachmentDescriptionArray = {renderpassAttachmentDescription};

    const auto attachmentReference      = vk::AttachmentReference{}.setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    std::array attachmentReferenceArray = {attachmentReference};

    const auto subpassDescription =
        vk::SubpassDescription{}.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setColorAttachments(attachmentReferenceArray);
    std::array subpassDescriptionArray = {subpassDescription};

    const auto renderpassCreateInfo =
        vk::RenderPassCreateInfo{}.setAttachments(renderpassAttachmentDescriptionArray).setSubpasses(subpassDescriptionArray);


    if (const auto result = mVulkanDevice->logicDevice().createRenderPass(renderpassCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkRenderPass = result.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create an RenderPass, result code \"" + vk::to_string(result.result) + "\"");
    }
}

void VulkanRenderer::createSyncStructure()
{
    const auto fenceCreateInfo = vk::FenceCreateInfo{}.setFlags(vk::FenceCreateFlagBits::eSignaled);
    if (const auto result = mVulkanDevice->logicDevice().createFence(fenceCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkRenderFence = result.value;
    }

    if (const auto result = mVulkanDevice->logicDevice().createSemaphore(vk::SemaphoreCreateInfo{}); result.result == vk::Result::eSuccess)
    {
        mVkRenderSemaphore = result.value;
    }

    if (const auto result = mVulkanDevice->logicDevice().createSemaphore(vk::SemaphoreCreateInfo{}); result.result == vk::Result::eSuccess)
    {
        mVkPresentSemaphore = result.value;
    }

    if (!mVkRenderFence || !mVkRenderSemaphore || !mVkPresentSemaphore)
    {
        Kompot::ErrorHandling::exit("Failed to create sync structures");
    }
}
void VulkanRenderer::draw(Window* window)
{
    auto windowAttributes = dynamic_cast<VulkanWindowRendererAttributes*>(window ? window->getWindowRendererAttributes() : nullptr);
    if (!windowAttributes)
    {
        return;
    }

    // wait until the GPU has finished rendering the last frame. Timeout of 1 second
    std::array fencesArray = {mVkRenderFence};
    const auto timeout     = std::chrono::microseconds(1).count();
    check(mVulkanDevice->logicDevice().waitForFences(fencesArray, true, timeout) == vk::Result::eSuccess);
    mVulkanDevice->logicDevice().resetFences(fencesArray);

    uint32_t swapchainImageIndex = 0;
    if (const auto result =
            mVulkanDevice->logicDevice().acquireNextImageKHR(windowAttributes->swapchain.handler, timeout, mVkPresentSemaphore, nullptr);
        result.result == vk::Result::eSuccess)
    {
        swapchainImageIndex = result.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to acquire next framebuffer image");
    }

    check(mMainCommandBuffer.reset(vk::CommandBufferResetFlagBits{}) == vk::Result::eSuccess);
    check(mMainCommandBuffer.begin(vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)) == vk::Result::eSuccess);

    const float flash              = abs(sin(mFrameNumber / 120.f));
    const auto clearValue          = vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, flash, 1.0f}));
    const auto renderPassBeginInfo = vk::RenderPassBeginInfo{}
                                         .setRenderPass(mVkRenderPass)
                                         .setFramebuffer(windowAttributes->swapchain.framebuffers[swapchainImageIndex])
                                         .setRenderArea(vk::Rect2D{}.setExtent(windowAttributes->scissor.extent))
                                         .setClearValueCount(1)
                                         .setPClearValues(&clearValue);

    mMainCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    mMainCommandBuffer.endRenderPass();

    check(mMainCommandBuffer.end() == vk::Result::eSuccess);

    const std::array renderSemaphoreArray    = {mVkRenderSemaphore};
    const std::array pipelineStageFlagsArray = {vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput}};
    const std::array commandBufferArray      = {mMainCommandBuffer};
    const std::array presentSemaphoreArray   = {mVkPresentSemaphore};
    const std::array submitInfoArray         = {vk::SubmitInfo{}
                                            .setWaitSemaphores(renderSemaphoreArray)
                                            .setWaitDstStageMask(pipelineStageFlagsArray)
                                            .setCommandBuffers(commandBufferArray)
                                            .setSignalSemaphores(presentSemaphoreArray)};

    check(mVulkanDevice->getGraphicsQueue().submit(submitInfoArray, mVkRenderFence) == vk::Result::eSuccess);

    const std::array swapchainArray    = {windowAttributes->swapchain.handler};
    const std::array ImageIndicesArray = {swapchainImageIndex};
    const auto presentInfo =
        vk::PresentInfoKHR{}.setWaitSemaphores(renderSemaphoreArray).setSwapchains(swapchainArray).setImageIndices(ImageIndicesArray);

    check(windowAttributes->presentQueue.presentKHR(presentInfo) == vk::Result::eSuccess);

    ++mFrameNumber;
}
