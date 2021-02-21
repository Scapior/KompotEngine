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

bool VulkanShader::load()
{
    std::ifstream file("Shaders/"s + mFilename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Log::getInstance() << "Couldn't open shader file " << mFilename << std::endl;
        return false;
    }
    const auto fileSize = file.tellg();
    std::vector<char> buffer(static_cast<unsigned long>(fileSize));
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    if (mFilename.find(".frag") != std::string::npos)
    {
        mStageFlag = vk::ShaderStageFlagBits::eFragment;
    }
    else if (mFilename.find(".vert") != std::string::npos)
    {
        mStageFlag = vk::ShaderStageFlagBits::eVertex;
    }

    const auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo{}.setCodeSize(fileSize).setPCode(reinterpret_cast<const uint32_t*>(buffer.data()));

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
