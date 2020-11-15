/*
*  VulkanRenderer.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "VulkanRenderer.hpp"
#include <vulkan/vulkan.hpp>

using namespace Kompot;

VulkanRenderer::VulkanRenderer()
{
    vk::DispatchLoaderStatic Loader;

    vk::createInstance(nullptr, nullptr, nullptr, Loader);
}
