#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>

namespace KompotEngine
{

namespace Renderer
{

class Buffer
{
public:
    Buffer(VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize);
    VkDeviceSize getSize() const;
    VkBuffer &getBuffer();
    VkResult copyFromRawPointer(const void*, VkDeviceSize);
    VkDeviceMemory getBufferMemory() const;
    ~Buffer();
private:
    VkDevice       m_vkDevice;
    VkBuffer       m_vkBuffer;
    VkDeviceMemory m_vkBufferMemory;
    VkDeviceSize   m_vkBufferMemorySize;

};


} // namespace Renderer

} // namespace KompotEngine
