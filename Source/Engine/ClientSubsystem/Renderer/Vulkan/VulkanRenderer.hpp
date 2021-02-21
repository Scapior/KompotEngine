/*
 *  VulkanRenderer.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once
#include "VulkanShader.hpp"
#include "../IRenderer.hpp"
#include "VulkanTypes.hpp"
#include "VulkanDevice.hpp"
#include "VulkanPipelineBuilder.hpp"
#include <Memory/VulkanAllocator/VulkanAllocator.hpp>
#include <vulkan/vulkan.hpp>
#include <set>

namespace Kompot
{
class VulkanRenderer : public Kompot::IRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer();

    void draw(Window* window) override;

    void notifyWindowResized(Window* window) override;
    WindowRendererAttributes* updateWindowAttributes(Window* window) override;
    void unregisterWindow(Window* window) override;

    std::string_view getName() const override
    {
        return "Vulkan";
    }

    const vk::Instance getVkInstance() const
    {
        return mVkInstance;
    };

    const vk::AllocationCallbacks getAllocatorCallbacks() const
    {
        return mAllocator;
    };

    vk::RenderPass getRenderPass() const
    {
        return mVkRenderPass;
    };

protected:
    void cleanupWindowHandlers(VulkanWindowRendererAttributes* windowAttributes);
    void recreateWindowHandlers(VulkanWindowRendererAttributes* windowAttributes);

private:
    void setupDebugCallback();
    void deleteDebugCallback();

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

    static VkBool32 vulkanDebugCallbackImpl(
        vk::DebugUtilsMessageSeverityFlagsEXT messageSeverite,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT& callbackData,
        [[maybe_unused]] void* userData);

private:
    std::size_t mFrameNumber = 0;

    VmaAllocator_T* mAllocator = nullptr;

    vk::Instance mVkInstance;
    std::unique_ptr<VulkanDevice> mVulkanDevice;

    VulkanPipelineBuilder mVulkanPipelineBuilder;

    vk::CommandPool mVkCommandPool;
    vk::CommandBuffer mMainCommandBuffer;

    vk::Format mVkSwapchainFormat = vk::Format::eB8G8R8A8Srgb; // ToDo: add selection based on GPU capabilities
    vk::RenderPass mVkRenderPass;
    std::vector<vk::Framebuffer> mVkFramebuffers;

    vk::Semaphore mVkPresentSemaphore;
    vk::Semaphore mVkRenderSemaphore;
    vk::Fence mVkRenderFence;

    VulkanShader mVertexShader;
    VulkanShader mFragmentShader;

    std::set<Window*> mWindows;

#if 1 // ENGINE_DEBUG
    vk::DebugUtilsMessengerEXT mVkDebugUtilsMessenger;
#endif

private:
    void setupAllocator();

    void createInstance();
    vk::PhysicalDevice selectPhysicalDevice();
    void createDevice();
    void createCommands();
    void createPipeline(VulkanWindowRendererAttributes* window);
    void createRenderpass();
    void createSyncStructure();
};

} // namespace Kompot
