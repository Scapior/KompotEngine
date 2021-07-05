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
    uint32_t         swapchainImagesCount;

    std::vector<vk::Image>          images;
    std::vector<vk::ImageView>      imageViews;
    std::vector<vk::Framebuffer>    framebuffers;
};

struct VulkanPipeline
{
    vk::PipelineLayout  pipelineLayout;
    vk::Pipeline        pipeline;
};

struct VulkanWindowRendererAttributes : public WindowRendererAttributes
{    
    vk::SurfaceKHR  surface;
    VulkanSwapchain swapchain;
    vk::Rect2D      scissor;
    vk::Queue       presentQueue;
    VulkanPipeline  pipeline;
    bool            isRenderingIdle    = false;
    bool            framebufferResized = false;
    bool            isPendingDestroy   = false;
};

struct VulkanFrameData
{
    vk::CommandPool   vkCommandPool;
    vk::CommandBuffer vkCommandBuffer;
    vk::Semaphore     vkPresentSemaphore;
    vk::Semaphore     vkRenderSemaphore;
    vk::Fence         vkRenderFence;
};

} // namespace Kompot
