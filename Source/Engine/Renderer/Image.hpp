#pragma once

#include "global.hpp"
#include "SingleTimeCommandBuffer.hpp"
#include <vulkan/vulkan.hpp>

namespace KompotEngine
{

namespace Renderer
{

class Image
{
public:
    Image(VkDevice, VkImage, VkImageView); // for swapchain images
    Image(VkDevice, VkExtent2D, VkImage, VkImageView, VkSampler, VkFormat, VkImageLayout, VkDeviceMemory, VkImageAspectFlags, uint32_t);
    ~Image();
    void transitImageLayout(VkImageLayout, SingleTimeCommandBuffer);

    VkImage     getImage() const;
    VkImageView getImageView() const;
    VkSampler   getSampler() const;
    VkExtent2D  getExtent() const;
    uint32_t    getMipLevelsCount() const;

private:
    VkDevice       m_vkDevice;
    VkExtent2D     m_vkExtent;
    VkImage        m_vkImage;
    VkImageView    m_vkImageView;
    VkSampler      m_vkImageSampler;
    VkFormat       m_vkImageFormat;
    VkImageLayout  m_vkCurrentImageLayout;
    VkDeviceMemory m_vkImageMemory;
    VkImageAspectFlags m_vkImageAspectFlags;
    uint32_t       m_mipLevelsCount;
};

} //namespace Renderer

} // namespace KompotEngine
