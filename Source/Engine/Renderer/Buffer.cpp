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

VkResult Buffer::copyFromRawPointer(const void *data, VkDeviceSize dataSize)
{
    if (m_vkBufferMemorySize < dataSize)
    {
        return VK_RESULT_RANGE_SIZE;
    }
    void* bufferMemoryMapPointer;
    auto resultCode = vkMapMemory(m_vkDevice, m_vkBufferMemory, 0_u64t, m_vkBufferMemorySize, 0_u32t, &bufferMemoryMapPointer);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Buffer::copyFromRawPointer(): Function vkMapMemory call failed with a code " << resultCode << "."<< std::endl;
        return resultCode;
    }
    memcpy(bufferMemoryMapPointer, data, m_vkBufferMemorySize);
    vkUnmapMemory(m_vkDevice, m_vkBufferMemory);
    return VK_SUCCESS;
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
