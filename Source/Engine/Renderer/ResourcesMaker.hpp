#pragma once

#include "global.hpp"
#include "Buffer.hpp"
#include "SingleTimeCommandBuffer.hpp"
#include <vulkan/vulkan.hpp>
#include <memory>

namespace KompotEngine
{

namespace Renderer
{

class ResourcesMaker
{
public:
    ResourcesMaker(VkPhysicalDevice, VkDevice, VkCommandPool, VkQueue);

    SingleTimeCommandBuffer createSingleTimeCommandBuffer() const;

    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags) const;
    std::shared_ptr<Buffer> createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags) const;
    std::shared_ptr<Buffer> createBufferCopy(const std::shared_ptr<Buffer>&, VkBufferUsageFlags, VkMemoryPropertyFlags) const;

private:
    VkDevice         m_vkDevice;
    VkPhysicalDevice m_vkPhysicalDevice;
    VkCommandPool    m_vkCommandPool;
    VkQueue          m_vkGraphicsQueue;


    VkBuffer createVkBuffer(VkDeviceSize, VkBufferUsageFlags) const;
    VkDeviceMemory allocateAndBindVkBufferMemory(VkBuffer, VkMemoryPropertyFlags) const;
};


} // namespace Renderer

} // namespace KompotEngine


