/*
 *  VulkanShader.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "VulkanShader.hpp"
#include <Engine/Log/Log.hpp>
#include <Engine/ErrorHandling.hpp>

using namespace Kompot;

VulkanShader::VulkanShader()
{
}

VulkanShader::VulkanShader(const VulkanShader& otherShader) :
    mFilename(otherShader.mFilename), mShaderModule(otherShader.mShaderModule), mStageFlag(otherShader.mStageFlag)
{
    mFilename     = otherShader.mFilename;
    mDevice       = otherShader.mDevice;
    mShaderModule = otherShader.mShaderModule;
    mStageFlag    = otherShader.mStageFlag;
}

VulkanShader::VulkanShader(const std::string_view& filename, vk::Device device) : mFilename(filename), mDevice(device)
{
}

VulkanShader::VulkanShader(VulkanShader&& otherShader)
{
    *this = otherShader;

    otherShader.mFilename     = std::string{};
    otherShader.mDevice       = nullptr;
    otherShader.mShaderModule = nullptr;
}

void VulkanShader::operator=(const VulkanShader& otherShader)
{
    mFilename     = otherShader.mFilename;
    mDevice       = otherShader.mDevice;
    mShaderModule = otherShader.mShaderModule;
    mStageFlag    = otherShader.mStageFlag;
}

VulkanShader::~VulkanShader()
{
//    if (mDevice && mShaderModule)
//    {
//        mDevice.destroy(mShaderModule);
//    }

    mFilename     = std::string{};
    mDevice       = nullptr;
    mShaderModule = nullptr;
}

bool VulkanShader::load(const std::vector<std::byte>& shaderBytecode)
{
    const auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo{}.setCodeSize(shaderBytecode.size()).setPCode(reinterpret_cast<const uint32_t*>(shaderBytecode.data()));

    if (const auto result = mDevice.createShaderModule(shaderModuleCreateInfo); result.result == vk::Result::eSuccess)
    {
        mShaderModule = result.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create Shader from file \"" + mFilename + "\", result code \"" + vk::to_string(result.result) + "\"");
    }

    return mShaderModule;
}
