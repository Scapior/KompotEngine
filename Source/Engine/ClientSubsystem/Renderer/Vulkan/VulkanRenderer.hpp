/*
 *  VulkanRenderer.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include "../IRenderer.hpp"
#include "VulkanDevice.hpp"
#include <set>
#include <vulkan/vulkan.hpp>

namespace Kompot
{

struct VulkanWindowRendererAttributes : WindowRendererAttributes
{
    vk::SurfaceKHR surface;
    vk::Rect2D scissor;
};

class VulkanRenderer : public Kompot::IRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer();

    WindowRendererAttributes* updateWindowAttributes(Window* window) override;
    void unregisterWindow(Window* window) override;

private:
    vk::Instance mVkInstance;
    std::unique_ptr<VulkanDevice> mVulkanDevice;

    void createInstance();
    vk::PhysicalDevice selectPhysicalDevice();
    void createDevice();

    std::set<Window*> mWindows;
};

} // namespace Kompot
