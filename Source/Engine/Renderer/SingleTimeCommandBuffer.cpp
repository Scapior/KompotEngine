#include "SingleTimeCommandBuffer.hpp"

using namespace KompotEngine::Renderer;

SingleTimeCommandBuffer::SingleTimeCommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, VkQueue vkGraphicsQueue)
    : m_vkDevice(vkDevice), m_vkCommandPool(vkCommandPool), m_vkGraphicsQueue(vkGraphicsQueue), m_vkCommandBuffer(nullptr)
{
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo = {};
    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandPool = m_vkCommandPool;
    vkCommandBufferAllocateInfo.commandBufferCount = 1_u32t;

    VkResult resultCode = vkAllocateCommandBuffers(m_vkDevice, &vkCommandBufferAllocateInfo, &m_vkCommandBuffer);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "SingleTimeCommandBuffer(..): Function vkAllocateCommandBuffers call failed with a code " << resultCode << "." << std::endl;
        m_vkCommandPool = nullptr;
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo = {};
    vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    resultCode = vkBeginCommandBuffer(m_vkCommandBuffer, &vkCommandBufferBeginInfo);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "SingleTimeCommandBuffer(..): Function vkBeginCommandBuffer call failed with a code " << resultCode << "." << std::endl;
        free();
    }
}

KompotEngine::Renderer::SingleTimeCommandBuffer::operator VkCommandBuffer() const
{
    return m_vkCommandBuffer;
}

VkResult SingleTimeCommandBuffer::submit()
{
    if (!m_vkCommandBuffer)
    {
        return VK_INCOMPLETE;
    }

    VkResult resultCode = vkEndCommandBuffer(m_vkCommandBuffer);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "SingleTimeCommandBuffer::submit(): Function vkEndCommandBuffer call failed with a code " << resultCode << "." << std::endl;
        free();
        return resultCode;
    }

    VkSubmitInfo vkSubmitInfo = {};
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &m_vkCommandBuffer;

    resultCode = vkQueueSubmit(m_vkGraphicsQueue, 1, &vkSubmitInfo, nullptr);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "SingleTimeCommandBuffer::submit(): Function vkQueueSubmit call failed with a code " << resultCode << "." << std::endl;
        free();
        return resultCode;
    }

    resultCode = vkQueueWaitIdle(m_vkGraphicsQueue);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "SingleTimeCommandBuffer::submit(): Function vkQueueSubmit call failed with a code " << resultCode << "." << std::endl;
        free();
        return resultCode;
    }

    free();
}

SingleTimeCommandBuffer::~SingleTimeCommandBuffer()
{
    if (m_vkCommandBuffer != nullptr)
    {
        this->submit();
    }
}

void SingleTimeCommandBuffer::free()
{
    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &m_vkCommandBuffer);
    m_vkCommandBuffer = nullptr;
}
