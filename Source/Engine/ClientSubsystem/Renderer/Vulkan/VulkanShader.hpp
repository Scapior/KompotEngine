/*
 *  VulkanShader.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once
#include "VulkanDevice.hpp"
#include <vulkan/vulkan.hpp>

namespace Kompot
{

class VulkanShader
{
public:
    VulkanShader();
    VulkanShader(const std::string_view& shaderFileName, vk::Device device);
    VulkanShader(const VulkanShader& shader);
    VulkanShader(VulkanShader&& otherShader);
    void operator=(const VulkanShader& otherShader);
    ~VulkanShader();

    bool load();

    vk::ShaderModule get() const
    {
        return mShaderModule;
    }

    std::string_view getSourceFilename() const
    {
        return mFilename;
    }

    vk::ShaderStageFlagBits getStageFlag() const
    {
        return mStageFlag;
    }

private:
    std::string mFilename;
    vk::Device mDevice;
    vk::ShaderModule mShaderModule;
    vk::ShaderStageFlagBits mStageFlag;
};

} // namespace Kompot