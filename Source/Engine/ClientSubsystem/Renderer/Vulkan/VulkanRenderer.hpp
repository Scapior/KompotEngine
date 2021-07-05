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

enum class RendererState
{
    Uninitialized,
    Initialized,
    DeviceLost
};

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
    void recreateWindowHandlers(VulkanWindowRendererAttributes* windowAttributes, vk::SurfaceCapabilitiesKHR vkSurfaceCapabilities);

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
    static const uint64_t VULKAN_BUFFERS_COUNT = 2;
    std::size_t mFrameNumber = 0;

    VmaAllocator_T* mAllocator = nullptr;

    vk::Instance mVkInstance;
    std::unique_ptr<VulkanDevice> mVulkanDevice;

    VulkanPipelineBuilder mVulkanPipelineBuilder;

    vk::Format mVkSwapchainFormat = vk::Format::eB8G8R8A8Srgb; // ToDo: add selection based on GPU capabilities
    vk::RenderPass mVkRenderPass;
    std::vector<vk::Framebuffer> mVkFramebuffers;

    std::array<VulkanFrameData, VULKAN_BUFFERS_COUNT> mVulkanFrames;

    VulkanShader mVertexShader;
    VulkanShader mFragmentShader;

    std::set<Window*> mWindows;

    RendererState mRendererState = RendererState::Uninitialized;

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
    void createSyncObjects();

    VulkanFrameData& getCurrentFrame()
    {
        return mVulkanFrames[mFrameNumber % VULKAN_BUFFERS_COUNT];
    }

};

} // namespace Kompot
