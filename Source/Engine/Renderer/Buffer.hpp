#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>

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
    VkDeviceMemory getBufferMemory() const;
    ~Buffer();
private:
    VkDevice       m_vkDevice;
    mutable VkBuffer       m_vkBuffer;
    mutable VkDeviceMemory m_vkBufferMemory;
    VkDeviceSize   m_vkBufferMemorySize;

};


} // namespace Renderer

} // namespace KompotEngine
