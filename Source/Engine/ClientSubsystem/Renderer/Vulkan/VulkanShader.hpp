/*
 *  VulkanShader.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once
#include "VulkanDevice.hpp"
#include <Engine/ClientSubsystem/Renderer/RenderingCommon.hpp>
#include <vulkan/vulkan.hpp>

namespace Kompot::Rendering::Vulkan
{
class VulkanShader : IShader
{
public:
    VulkanShader();
    VulkanShader(const std::string_view& shaderFileName, vk::Device device);
    VulkanShader(const VulkanShader& shader);
    VulkanShader(VulkanShader&& otherShader);
    void operator=(const VulkanShader& otherShader);
    ~VulkanShader();

    bool load(const std::vector<uint32_t>& shaderBytecode);

    operator bool() const
    {
        return mShaderModule;
    }

    vk::ShaderModule get() const
    {
        return mShaderModule;
    }

    std::string_view getSourceFilename() const override
    {
        return mFilename;
    }

    vk::ShaderStageFlagBits getStageFlag() const
    {
        return mStageFlag;
    }

    void setStageFlag(vk::ShaderStageFlagBits inFlag)
    {
        mStageFlag = inFlag;
    }

private:
    std::string mFilename;
    vk::Device mDevice;
    vk::ShaderModule mShaderModule;
    vk::ShaderStageFlagBits mStageFlag;
};

} // namespace Kompot
