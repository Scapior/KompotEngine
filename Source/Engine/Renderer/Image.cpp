#include "Image.hpp"

using namespace KompotEngine::Renderer;

Image::Image(VkDevice vkDevice,
             VkImage vkImage,
             VkImageView vkImageView)
    : m_vkDevice(vkDevice),
      m_vkImage(vkImage),
      m_vkImageView(vkImageView),
      m_vkImageSampler(nullptr),
      m_vkImageMemory(nullptr),
      m_isSwapChainImage(true)
{ }

Image::Image(VkDevice vkDevice,
             VkExtent2D vkExtent,
             VkImage vkImage,
             VkImageView vkImageView,
             VkSampler vkImageSampler,
             VkFormat vkFormat,
             VkImageLayout vkImageLayout,
             VkDeviceMemory vkImageMemory,
             VkImageAspectFlags vkImageAspectFlags,
             uint32_t mipLevelsCount)
    : m_vkDevice(vkDevice),
      m_vkExtent(vkExtent),
      m_vkImage(vkImage),
      m_vkImageView(vkImageView),
      m_vkImageSampler(vkImageSampler),
      m_vkImageFormat(vkFormat),
      m_vkCurrentImageLayout(vkImageLayout),
      m_vkImageMemory(vkImageMemory),
      m_vkImageAspectFlags(vkImageAspectFlags),
      m_mipLevelsCount(mipLevelsCount),
      m_isSwapChainImage(false)
{ }

Image::~Image()
{
    if (m_vkImageSampler)
    {
        vkDestroySampler(m_vkDevice, m_vkImageSampler, nullptr);
    }
    vkDestroyImageView(m_vkDevice, m_vkImageView, nullptr);
    if (!m_isSwapChainImage)
    {
        vkDestroyImage(m_vkDevice, m_vkImage, nullptr);
    }
    if (m_vkImageMemory)
    {
        vkFreeMemory(m_vkDevice, m_vkImageMemory, nullptr);
    }
}

void Image::transitImageLayout(VkImageLayout newImageLayout, SingleTimeCommandBuffer commandBuffer)
{
    if (m_vkCurrentImageLayout == newImageLayout)
    {
        return;
    }

    VkImageMemoryBarrier vkImageMemoryBarrier = {};
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.oldLayout = m_vkCurrentImageLayout;
    vkImageMemoryBarrier.newLayout = newImageLayout;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = m_vkImage;
    vkImageMemoryBarrier.subresourceRange.aspectMask = m_vkImageAspectFlags;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0_u32t;
    vkImageMemoryBarrier.subresourceRange.levelCount = m_mipLevelsCount;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0_u32t;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1_u32t;

    VkPipelineStageFlags vkPipelineSourceStage;
    VkPipelineStageFlags vkPipelineDestinationStage;

    if (m_vkCurrentImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0_u32t;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkPipelineSourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineDestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (m_vkCurrentImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkPipelineSourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineDestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (m_vkCurrentImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0_u32t;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        vkPipelineSourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineDestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (m_vkCurrentImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkPipelineSourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        vkPipelineDestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else
    {
        Log::getInstance() << "Renderer::transitionImageLayout(): Failed to transit image layout." << std::endl;
        std::terminate();
    }

    vkCmdPipelineBarrier(commandBuffer, vkPipelineSourceStage, vkPipelineDestinationStage, 0_u32t, 0_u32t, nullptr, 0_u32t, nullptr, 1_u32t, &vkImageMemoryBarrier);
    commandBuffer.submit();
    m_vkCurrentImageLayout = newImageLayout;
}

const VkImage &Image::getImage() const
{
    return m_vkImage;
}

VkImageView Image::getImageView() const
{
    return m_vkImageView;
}

VkSampler Image::getSampler() const
{
    return m_vkImageSampler;
}

VkExtent2D Image::getExtent() const
{
    return m_vkExtent;
}

uint32_t Image::getMipLevelsCount() const
{
    return m_mipLevelsCount;
}

VkImageAspectFlags Image::getImageAspectFlags() const
{
    return m_vkImageAspectFlags;
}

void Image::setCurrentLayout(VkImageLayout imageLayout)
{
    m_vkCurrentImageLayout = imageLayout;
}
