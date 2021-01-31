/*
 *  VulkanRenderer.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once
#include "../IRenderer.hpp"
#include "VulkanTypes.h"
#include "VulkanDevice.hpp"
#include <set>
#include <vulkan/vulkan.hpp>

namespace Kompot
{
class VulkanRenderer : public Kompot::IRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer();

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

protected:
    void cleanupWindowSwapchain(VulkanWindowRendererAttributes* windowAttributes);
    void recreateSwapchain(VulkanWindowRendererAttributes* windowAttributes);

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
    vk::Instance mVkInstance;
    std::unique_ptr<VulkanDevice> mVulkanDevice;

    void createInstance();
    vk::PhysicalDevice selectPhysicalDevice();
    void createDevice();

    std::set<Window*> mWindows;

#if 1 // ENGINE_DEBUG
    vk::DebugUtilsMessengerEXT mVkDebugUtilsMessenger;
#endif
};

} // namespace Kompot
