/*
 *  VulkanPipelineBuilder.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include "VulkanTypes.hpp"
#include "VulkanShader.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>

namespace Kompot::Rendering::Vulkan
{
class VulkanRenderer;

enum class PrimitiveRestartOption : uint8_t
{
    Enabled,
    Disabled
};

class VulkanPipelineBuilder
{
public:
    void setDevice(vk::Device device);
    vk::Result buildGraphicsPipeline(
            VulkanWindowRendererAttributes* windowRendererAttributes,
            VulkanRenderer* renderer,
            const std::vector<VulkanShader>& shaders);

protected: // static create info builders
    static vk::PipelineShaderStageCreateInfo createPipelineShaderStage(
            const VulkanShader& shaderModule,
            const std::string_view& entryPointName = "main");

    // static vk::PipelineVertexInputStateCreateInfo createVertexInputStateCreateInfo(foo);

    static vk::PipelineInputAssemblyStateCreateInfo createInputAssemblyStateCreateInfo(
            vk::PrimitiveTopology topology,
            PrimitiveRestartOption primitiveRestartOption);

    static vk::PipelineViewportStateCreateInfo createViewportStateCreateInfo(const vk::Viewport& viewport, vk::Rect2D* viewportExtent);

    static vk::PipelineRasterizationStateCreateInfo createRasterizationStateCreateInfo(
            vk::PolygonMode polygonMode,
            vk::CullModeFlagBits cullMode,
            vk::FrontFace frontFace);

    static vk::PipelineMultisampleStateCreateInfo createMultisampleStateCreateInfo(vk::SampleCountFlagBits samples);

    static vk::PipelineColorBlendAttachmentState createPipelineColorBlendAttachmentState();
    static vk::PipelineColorBlendStateCreateInfo createColorBlendStateCreateInfo(
            const vk::PipelineColorBlendAttachmentState* pipelineColorBlendAttachmentState);


    static vk::PipelineLayoutCreateInfo createLayoutCreateInfo();

private:
    vk::Device mDevice;


    //    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    //    VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
    //    VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
    //    VkViewport _viewport;
    //    VkRect2D _scissor;
    //    VkPipelineRasterizationStateCreateInfo _rasterizer;
    //    VkPipelineColorBlendAttachmentState _colorBlendAttachment;
    //    VkPipelineMultisampleStateCreateInfo _multisampling;
    //    VkPipelineLayout _pipelineLayout;
};

} // namespace Kompot
