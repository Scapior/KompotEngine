/*
 *  VulkanRenderer.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#define VMA_IMPLEMENTATION

#include "VulkanRenderer.hpp"
#include <Engine/ClientSubsystem/Window/Window.hpp>
#include <Engine/ErrorHandling.hpp>
#include <Engine/Log/Log.hpp>
#include <EngineDefines.hpp>
#include <vector>
#include <cmath>

#define ENGINE_VULKAN_VERSION VK_API_VERSION_1_2

using namespace Kompot;

//const uint64_t VulkanRenderer::VULKAN_BUFFERS_COUNT = 2;

VulkanRenderer::VulkanRenderer() : mVkInstance(nullptr)
{
    createInstance();
    setupDebugCallback();
    mVulkanDevice.reset(new VulkanDevice(mVkInstance, selectPhysicalDevice()));
    mVulkanPipelineBuilder.setDevice(mVulkanDevice->asLogicDevice());

    setupAllocator();

    createCommands();
    createRenderpass();
    createSyncObjects();
};

VulkanRenderer::~VulkanRenderer()
{
    mVulkanDevice->asLogicDevice().destroy(mVertexShader.get());
    mVulkanDevice->asLogicDevice().destroy(mFragmentShader.get());

    for (auto& frame : mVulkanFrames)
    {
        mVulkanDevice->asLogicDevice().destroy(frame.vkRenderFence);
        mVulkanDevice->asLogicDevice().destroy(frame.vkRenderSemaphore);
        mVulkanDevice->asLogicDevice().destroy(frame.vkPresentSemaphore);

        mVulkanDevice->asLogicDevice().destroy(frame.vkCommandPool);
        frame.vkCommandBuffer = nullptr;
    }

    mVulkanDevice->asLogicDevice().destroy(mVkRenderPass);
    // mVulkanDevice->asLogicDevice().destroy();
    mVulkanDevice.reset();
    deleteDebugCallback();
    mVkInstance.destroy(nullptr);
}

void VulkanRenderer::createInstance()
{
    vk::ApplicationInfo vkApplicationInfo{ENGINE_NAME, 0, ENGINE_NAME, 0, ENGINE_VULKAN_VERSION};

    const auto instanceExtensions       = VulkanUtils::getRequiredInstanceExtensions();
    const auto instanceValidationLayers = VulkanUtils::getRequiredInstanceValidationLayers();
    auto vkInstanceCreateInfo           = vk::InstanceCreateInfo{}
                                    .setPApplicationInfo(&vkApplicationInfo)
                                    .setPEnabledExtensionNames(instanceExtensions)
                                    .setPEnabledLayerNames(instanceValidationLayers)
                                    .setEnabledLayerCount(static_cast<uint32_t>(instanceValidationLayers.size()))
                                    .setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()));

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

    check(mVulkanDevice->asLogicDevice().waitIdle() == vk::Result::eSuccess);

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
    cleanupWindowHandlers(windowAttributes);
    std::array<uint32_t, 2> windowExtent;
    windowExtent                     = window->getExtent();
    windowAttributes->scissor.extent = vk::Extent2D{windowExtent[0], windowExtent[1]};
    if (windowAttributes && !windowAttributes->surface)
    {
        windowAttributes->surface = window->createVulkanSurface();
    }
    // const auto windowExtentAfter = window->getExtent();
    recreateWindowHandlers(windowAttributes);
    Log::getInstance() << "updateWindowAttributes" << std::endl;
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
        vulkanWindowAttributes->isPendingDestroy = true;
        check(mVulkanDevice->asLogicDevice().waitIdle() == vk::Result::eSuccess);

        cleanupWindowHandlers(vulkanWindowAttributes);
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

    if (auto surfaceCapabilitiesResult = mVulkanDevice->asPhysicalDevice().getSurfaceCapabilitiesKHR(windowAttributes->surface);
        surfaceCapabilitiesResult.result == vk::Result::eSuccess)
    {
        auto& vkSurfaceCapabilities = surfaceCapabilitiesResult.value;

        const auto foundPresentQueue = mVulkanDevice->findPresentQueue(windowAttributes);
        const auto graphicQueueIndex = mVulkanDevice->getGraphicsQueueIndex();

        windowAttributes->presentQueue                   = foundPresentQueue.first;
        const std::array<uint32_t, 2> queueFamilyIndices = {foundPresentQueue.second, graphicQueueIndex};
        const bool bQueuesAreSame                        = graphicQueueIndex == foundPresentQueue.second;

        windowAttributes->scissor.setExtent(vkSurfaceCapabilities.currentExtent);

        // VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR  ?
        auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
                                       .setSurface(windowAttributes->surface)
                                       .setMinImageCount(vkSurfaceCapabilities.minImageCount)
                                       .setImageFormat(mVkSwapchainFormat)
                                       .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
                                       .setImageExtent(vkSurfaceCapabilities.currentExtent)
                                       .setImageArrayLayers(1)
                                       .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                                       .setImageSharingMode(bQueuesAreSame ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent)
                                       .setQueueFamilyIndices(queueFamilyIndices)
                                       .setPreTransform(vkSurfaceCapabilities.currentTransform)
                                       .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                                       .setPresentMode(vk::PresentModeKHR::eMailbox)
                                       .setClipped(VK_TRUE);
        //.setOldSwapchain(windowAttributes->swapchain.handler);

        const auto& logicalDevice = mVulkanDevice->asLogicDevice();
        auto& swapchain           = windowAttributes->swapchain;
        if (const auto result = logicalDevice.createSwapchainKHR(swapchainCreateInfo); result.result == vk::Result::eSuccess)
        {
            swapchain.handler = result.value;
        }
        if (const auto result = logicalDevice.getSwapchainImagesKHR(swapchain.handler); result.result == vk::Result::eSuccess)
        {
            swapchain.images = result.value;
        }
        swapchain.swapchainImagesCount = swapchain.images.size();
        swapchain.imageViews.resize(swapchain.swapchainImagesCount);

        swapchain.framebuffers.resize(swapchain.swapchainImagesCount);
        auto framebufferCreateInfo = vk::FramebufferCreateInfo{}
                                         .setRenderPass(mVkRenderPass)
                                         .setHeight(windowAttributes->scissor.extent.height)
                                         .setWidth(windowAttributes->scissor.extent.width)
                                         .setLayers(1)
                                         .setAttachmentCount(1);

        for (std::size_t i = 0; i < swapchain.swapchainImagesCount; ++i)
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
                    "Failed to create a framebuffer for swapchain image, result code \"" + vk::to_string(result.result) + "\"");
            }
        }
    }

    createPipeline(windowAttributes);

    windowAttributes->framebufferResized = false;
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

void VulkanRenderer::cleanupWindowHandlers(VulkanWindowRendererAttributes* windowAttributes)
{
    if (!windowAttributes)
    {
        return;
    }

    auto& logicDevice = mVulkanDevice->asLogicDevice();

    logicDevice.destroy(windowAttributes->pipeline.pipelineLayout);
    logicDevice.destroy(windowAttributes->pipeline.pipeline);

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
}

void VulkanRenderer::createCommands()
{
    const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo{}
                                           .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                           .setQueueFamilyIndex(mVulkanDevice->getGraphicsQueueIndex());

    for (auto& frame : mVulkanFrames)
    {

        if (const auto result = mVulkanDevice->asLogicDevice().createCommandPool(commandPoolCreateInfo); result.result == vk::Result::eSuccess)
        {
            frame.vkCommandPool = result.value;
        }
        else
        {
            Kompot::ErrorHandling::exit("Failed to create a CommandPool, result code \"" + vk::to_string(result.result) + "\"");
        }

        const auto commandBufferAllocateInfo =
            vk::CommandBufferAllocateInfo{}.setCommandPool(frame.vkCommandPool).setCommandBufferCount(1).setLevel(vk::CommandBufferLevel::ePrimary);

        if (const auto result = mVulkanDevice->asLogicDevice().allocateCommandBuffers(commandBufferAllocateInfo);
            result.result == vk::Result::eSuccess && result.value.size() == 1)
        {
            frame.vkCommandBuffer = result.value[0];
        }
        else
        {
            Kompot::ErrorHandling::exit("Failed to create a CommandBuffer, result code \"" + vk::to_string(result.result) + "\"");
        }
    }
}

void VulkanRenderer::createRenderpass()
{
    const auto renderpassAttachmentDescription = vk::AttachmentDescription{}
                                                     .setFormat(mVkSwapchainFormat)
                                                     .setSamples(vk::SampleCountFlagBits::e1)
                                                     .setLoadOp(vk::AttachmentLoadOp::eClear)
                                                     .setStoreOp(vk::AttachmentStoreOp::eStore)
                                                     .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                                                     .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                                     .setInitialLayout(vk::ImageLayout::eUndefined)
                                                     .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    const auto attachmentReference = vk::AttachmentReference{}.setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    const auto subpassDescription = vk::SubpassDescription{}
                                        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                        .setColorAttachmentCount(1)
                                        .setPColorAttachments(&attachmentReference);

    const auto renderpassCreateInfo = vk::RenderPassCreateInfo{}
                                          .setAttachmentCount(1)
                                          .setPAttachments(&renderpassAttachmentDescription)
                                          .setSubpassCount(1)
                                          .setPSubpasses(&subpassDescription);


    if (const auto result = mVulkanDevice->asLogicDevice().createRenderPass(renderpassCreateInfo); result.result == vk::Result::eSuccess)
    {
        mVkRenderPass = result.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create a RenderPass, result code \"" + vk::to_string(result.result) + "\"");
    }
}

void VulkanRenderer::createSyncObjects()
{
    const auto fenceCreateInfo = vk::FenceCreateInfo{}.setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (auto& frame : mVulkanFrames)
    {
        if (const auto result = mVulkanDevice->asLogicDevice().createFence(fenceCreateInfo); result.result == vk::Result::eSuccess)
        {
            frame.vkRenderFence = result.value;
        }

        if (const auto result = mVulkanDevice->asLogicDevice().createSemaphore(vk::SemaphoreCreateInfo{}); result.result == vk::Result::eSuccess)
        {
            frame.vkRenderSemaphore = result.value;
        }

        if (const auto result = mVulkanDevice->asLogicDevice().createSemaphore(vk::SemaphoreCreateInfo{}); result.result == vk::Result::eSuccess)
        {
            frame.vkPresentSemaphore = result.value;
        }

        if (!frame.vkRenderFence || !frame.vkRenderSemaphore || !frame.vkPresentSemaphore)
        {
            Kompot::ErrorHandling::exit("Failed to create sync structures");
        }
    }
}

void VulkanRenderer::draw(Window* window)
{
    auto windowAttributes = dynamic_cast<VulkanWindowRendererAttributes*>(window ? window->getWindowRendererAttributes() : nullptr);
    if (!windowAttributes || windowAttributes->isPendingDestroy)
    {
        return;
    }

    auto currentFrame = getCurrentFrame();

    // wait until the GPU has finished rendering the last frame. Timeout of 1 second
    static const auto timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();
    check(mVulkanDevice->asLogicDevice().waitForFences(1, &currentFrame.vkRenderFence, true, timeout) == vk::Result::eSuccess);
    check(mVulkanDevice->asLogicDevice().resetFences(1, &currentFrame.vkRenderFence) == vk::Result::eSuccess);

    check(currentFrame.vkCommandBuffer.reset(vk::CommandBufferResetFlagBits{}) == vk::Result::eSuccess);

    uint32_t swapchainImageIndex = 0;
    if (const auto result =
            mVulkanDevice->asLogicDevice().acquireNextImageKHR(windowAttributes->swapchain.handler, timeout, currentFrame.vkPresentSemaphore, nullptr);
        result.result == vk::Result::eSuccess)
    {
        swapchainImageIndex = result.value;
    }
    else if (result.result == vk::Result::eErrorOutOfDateKHR || result.result == vk::Result::eSuboptimalKHR || windowAttributes->framebufferResized)
    {
        updateWindowAttributes(window);
        return;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to acquire next framebuffer image");
    }

    Log::getInstance() << "Draw frame #" << mFrameNumber << std::endl;

    check(currentFrame.vkCommandBuffer.begin(vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)) == vk::Result::eSuccess);

    const float flash              = std::abs(std::sin(mFrameNumber / 120.f));
    const auto clearValue          = vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, flash, 1.0f}));
    const auto renderPassBeginInfo = vk::RenderPassBeginInfo{}
                                         .setRenderPass(mVkRenderPass)
                                         .setFramebuffer(windowAttributes->swapchain.framebuffers[swapchainImageIndex])
                                         .setRenderArea(vk::Rect2D{}.setExtent(windowAttributes->scissor.extent))
                                         .setClearValueCount(1)
                                         .setPClearValues(&clearValue);

    currentFrame.vkCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    currentFrame.vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, windowAttributes->pipeline.pipeline);
    currentFrame.vkCommandBuffer.draw(3, 1, 0, 0);

    currentFrame.vkCommandBuffer.endRenderPass();

    check(currentFrame.vkCommandBuffer.end() == vk::Result::eSuccess);

    const auto pipelineStageFlags = vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const auto submitInfo         = vk::SubmitInfo{}
                                .setWaitSemaphoreCount(1)
                                .setPWaitSemaphores(&currentFrame.vkPresentSemaphore)
                                .setPWaitDstStageMask(&pipelineStageFlags)
                                .setCommandBufferCount(1)
                                .setPCommandBuffers(&currentFrame.vkCommandBuffer)
                                .setSignalSemaphoreCount(1)
                                .setPSignalSemaphores(&currentFrame.vkRenderSemaphore);

    check(mVulkanDevice->getGraphicsQueue().submit(1, &submitInfo, currentFrame.vkRenderFence) == vk::Result::eSuccess);

    const auto presentInfo = vk::PresentInfoKHR{}
                                 .setWaitSemaphoreCount(1)
                                 .setPWaitSemaphores(&currentFrame.vkRenderSemaphore)
                                 .setSwapchainCount(1)
                                 .setPSwapchains(&windowAttributes->swapchain.handler)
                                 .setPImageIndices(&swapchainImageIndex);

    const auto presentResult = windowAttributes->presentQueue.presentKHR(presentInfo);
    if (presentResult == vk::Result::eErrorOutOfDateKHR)
    {
        windowAttributes->framebufferResized = true;
    }
    if (presentResult != vk::Result::eSuccess)
    {
        Log::getInstance() << "presentResult = " << vk::to_string(presentResult) << std::endl;
    }

    ++mFrameNumber;
}

void VulkanRenderer::notifyWindowResized(Window* window)
{
    auto windowAttributes = dynamic_cast<VulkanWindowRendererAttributes*>(window ? window->getWindowRendererAttributes() : nullptr);
    if (windowAttributes)
    {
        windowAttributes->framebufferResized = true;
    }
}

void VulkanRenderer::setupAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = ENGINE_VULKAN_VERSION;
    allocatorInfo.physicalDevice   = mVulkanDevice->asPhysicalDevice();
    allocatorInfo.device           = mVulkanDevice->asLogicDevice();
    allocatorInfo.instance         = mVkInstance;
    // allocatorInfo.frameInUseCount = get from phys dev cap ?

    if (const auto result = vmaCreateAllocator(&allocatorInfo, &mAllocator); result != VK_SUCCESS)
    {
        Kompot::ErrorHandling::exit("Failed to create a VulkanMemoryAllocator, result code \"" + vk::to_string(vk::Result(result)) + "\"");
    }
}

void VulkanRenderer::createPipeline(VulkanWindowRendererAttributes* windowAttributes)
{
    if (!windowAttributes)
    {
        return;
    }

    if (!mVertexShader.get())
    {
        mVertexShader = VulkanShader("triangle.vert.spv", mVulkanDevice->asLogicDevice());
    }
    if (!mFragmentShader.get())
    {
        mFragmentShader = VulkanShader("triangle.frag.spv", mVulkanDevice->asLogicDevice());
    }

    if (!mVertexShader.load() || !mFragmentShader.load())
    {
        //Log::getInstance() << "Failed to create shaders" << std::endl;
    }

    std::vector<VulkanShader> shaders = {mVertexShader, mFragmentShader};
    if (const auto result = mVulkanPipelineBuilder.buildGraphicsPipeline(windowAttributes, this, shaders); result != vk::Result::eSuccess)
    {
        Kompot::ErrorHandling::exit("Failed to build graphics pipeline");
    }
}
