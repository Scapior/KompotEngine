/*
 *  VulkanPipelineBuilder.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "VulkanPipelineBuilder.hpp"
#include "VulkanRenderer.hpp"
#include <Engine/Log/Log.hpp>
#include <algorithm>

using namespace Kompot;
using namespace Kompot::Rendering::Vulkan;

void VulkanPipelineBuilder::setDevice(vk::Device device)
{
    mDevice = device;
}

vk::Result VulkanPipelineBuilder::buildGraphicsPipeline(
        VulkanWindowRendererAttributes* windowRendererAttributes,
        VulkanRenderer* renderer,
        const std::vector<VulkanShader>& shaders)
{
    VulkanPipeline pipeline{};

    if (!windowRendererAttributes || !renderer)
    {
        return vk::Result::eErrorUnknown;
    }

    // ensure that each shader is valid and intended for own stage
    std::set<vk::ShaderStageFlagBits> shaderStagesFlags;
    std::transform(shaders.cbegin(), shaders.cend(), std::inserter(shaderStagesFlags, shaderStagesFlags.begin()), [](const VulkanShader& shader) {
        return shader.getStageFlag();
    });

    bool haveInvalidShader = false;
    for (const auto& shader : shaders)
    {
        if (!shader.get())
        {
            haveInvalidShader = true;
        }
    }

    if (shaderStagesFlags.size() != shaders.size() || haveInvalidShader)
    {
        Log::getInstance() << "Tried to build a graphics pipeline with shaders of equal stages" << std::endl;
        return vk::Result::eErrorUnknown;
    }

    // build shaders stages
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    for (const auto& shader : shaders)
    {
        shaderStages.emplace_back(createPipelineShaderStage(shader));
    }

    // pipelien stages
    const vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};

    const auto inputAssemblyStateCreateInfo =
            createInputAssemblyStateCreateInfo(vk::PrimitiveTopology::eTriangleList, PrimitiveRestartOption::Disabled);

    const auto viewport = vk::Viewport{}
            .setWidth(static_cast<float>(windowRendererAttributes->scissor.extent.width))
            .setHeight(static_cast<float>(windowRendererAttributes->scissor.extent.height))
            .setMaxDepth(1.0f);
    const auto viewportStateCreateInfo = createViewportStateCreateInfo(viewport, &windowRendererAttributes->scissor);

    const auto rasterizationStateCreateInfo =
            createRasterizationStateCreateInfo(vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise);

    const auto multisampleStateCreateInfo = createMultisampleStateCreateInfo(vk::SampleCountFlagBits::e1);

    //    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStageCreateInfo = {};
    //    vkPipelineDepthStencilStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //    vkPipelineDepthStencilStageCreateInfo.depthTestEnable = VK_TRUE;
    //    vkPipelineDepthStencilStageCreateInfo.depthWriteEnable = VK_TRUE;
    //    vkPipelineDepthStencilStageCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    //    vkPipelineDepthStencilStageCreateInfo.depthBoundsTestEnable = VK_FALSE;
    //    vkPipelineDepthStencilStageCreateInfo.stencilTestEnable = VK_FALSE;

    const auto colorBlendAttachment      = createPipelineColorBlendAttachmentState();
    const auto colorBlendStateCreateInfo = createColorBlendStateCreateInfo(&colorBlendAttachment);

    // VkPipelineDynamicStateCreateInfo

    if (const auto createPipelineLayoutResult = mDevice.createPipelineLayout(createLayoutCreateInfo());
            createPipelineLayoutResult.result == vk::Result::eSuccess)
    {
        pipeline.pipelineLayout = createPipelineLayoutResult.value;
    }
    else
    {
        Log::getInstance() << "Tried to build a graphics pipeline with shaders of equal stages" << std::endl;
        return vk::Result::eErrorUnknown;
    }

    // finally build pipeline
    const auto graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo{}
            .setStages(shaderStages)
            .setPVertexInputState(&pipelineVertexInputStateCreateInfo)
            .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
            // tessellation
            .setPViewportState(&viewportStateCreateInfo)
            .setPRasterizationState(&rasterizationStateCreateInfo)
            .setPMultisampleState(&multisampleStateCreateInfo)
            // VkPipelineDepthStencilStateCreateInfo
            .setPColorBlendState(&colorBlendStateCreateInfo)
            // VkPipelineDynamicStateCreateInfo
            .setLayout(pipeline.pipelineLayout)
            .setRenderPass(renderer->getRenderPass())
            .setSubpass(0);

    if (const auto createPipelineResult = mDevice.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
            createPipelineResult.result == vk::Result::eSuccess)
    {
        pipeline.pipeline = createPipelineResult.value;
    }
    else
    {
        mDevice.destroy(pipeline.pipelineLayout);
        Log::getInstance() << "Tried to build a graphics pipeline with shaders of equal stages" << std::endl;
        return vk::Result::eErrorUnknown;
    }

    windowRendererAttributes->pipeline = pipeline;

    return vk::Result::eSuccess;
}

vk::PipelineShaderStageCreateInfo VulkanPipelineBuilder::createPipelineShaderStage(
        const VulkanShader& shaderModule,
        const std::string_view& entryPointName)
{
    return vk::PipelineShaderStageCreateInfo{}.setModule(shaderModule.get()).setPName(entryPointName.data()).setStage(shaderModule.getStageFlag());
}

vk::PipelineInputAssemblyStateCreateInfo VulkanPipelineBuilder::createInputAssemblyStateCreateInfo(
        vk::PrimitiveTopology topology,
        PrimitiveRestartOption primitiveRestartOption)
{
    return vk::PipelineInputAssemblyStateCreateInfo().setTopology(topology).setPrimitiveRestartEnable(
                primitiveRestartOption == PrimitiveRestartOption::Enabled);
}

vk::PipelineViewportStateCreateInfo VulkanPipelineBuilder::createViewportStateCreateInfo(const vk::Viewport& viewport, vk::Rect2D* viewportExtent)
{
    return vk::PipelineViewportStateCreateInfo{}.setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(viewportExtent);
}

vk::PipelineRasterizationStateCreateInfo VulkanPipelineBuilder::createRasterizationStateCreateInfo(
        vk::PolygonMode polygonMode,
        vk::CullModeFlagBits cullMode,
        vk::FrontFace frontFace)
{
    return vk::PipelineRasterizationStateCreateInfo{}.setPolygonMode(polygonMode).setLineWidth(1.0f).setCullMode(cullMode).setFrontFace(frontFace);
}

vk::PipelineMultisampleStateCreateInfo VulkanPipelineBuilder::createMultisampleStateCreateInfo(vk::SampleCountFlagBits samples)
{
    return vk::PipelineMultisampleStateCreateInfo().setRasterizationSamples(samples);
}

vk::PipelineColorBlendAttachmentState VulkanPipelineBuilder::createPipelineColorBlendAttachmentState()
{
    return vk::PipelineColorBlendAttachmentState{}.setBlendEnable(false).setColorWriteMask(
    vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB);
}

vk::PipelineColorBlendStateCreateInfo VulkanPipelineBuilder::createColorBlendStateCreateInfo(
        const vk::PipelineColorBlendAttachmentState* pipelineColorBlendAttachmentState)
{
    return vk::PipelineColorBlendStateCreateInfo{}.setAttachmentCount(1).setPAttachments(pipelineColorBlendAttachmentState);
}

vk::PipelineLayoutCreateInfo VulkanPipelineBuilder::createLayoutCreateInfo()
{
    return vk::PipelineLayoutCreateInfo{};
}
