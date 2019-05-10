#include "Buffer.hpp"

using namespace KompotEngine::Renderer;

Buffer::Buffer(VkDevice vkDevice, VkBuffer vkBuffer, VkDeviceMemory vkBufferMemory, VkDeviceSize vkBufferMemorySize)
    : m_vkDevice(vkDevice), m_vkBuffer(vkBuffer), m_vkBufferMemory(vkBufferMemory), m_vkBufferMemorySize(vkBufferMemorySize) {}

VkDeviceSize Buffer::getSize() const
{
    return m_vkBufferMemorySize;
}

VkBuffer &Buffer::getBuffer()
{
    return m_vkBuffer;
}


VkDeviceMemory Buffer::getBufferMemory() const
{
    return m_vkBufferMemory;
}

Buffer::~Buffer()
{
    vkDestroyBuffer(m_vkDevice, m_vkBuffer, nullptr);
    vkFreeMemory(m_vkDevice, m_vkBufferMemory, nullptr);
}
