/*
 *  VulkanTypes.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include <Engine/ClientSubsystem/Renderer/IRenderer.hpp>
#include <vulkan/vulkan.hpp>
#include <vector>

namespace Kompot
{
struct VulkanSwapchain
{
    vk::SwapchainKHR handler;

    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;
    std::vector<vk::Framebuffer> framebuffers;
};

struct VulkanWindowRendererAttributes : public WindowRendererAttributes
{
    vk::SurfaceKHR surface;
    VulkanSwapchain swapchain;
    vk::Rect2D scissor;
    vk::Queue presentQueue;
    bool framebufferResized = false;
    bool isPendingDestroy   = false;
};

} // namespace Kompot