#pragma once

#include "global.hpp"
#include "Buffer.hpp"
#include <vulkan/vulkan.hpp>

namespace KompotEngine
{

namespace Renderer
{


class SingleTimeCommandBuffer
{
public:
    SingleTimeCommandBuffer(VkDevice, VkCommandPool, VkQueue);
    operator VkCommandBuffer() const;
    operator const VkCommandBuffer*() const;
    VkResult submit();
    void free();
    ~SingleTimeCommandBuffer();
private:
    VkDevice        m_vkDevice;
    VkQueue         m_vkGraphicsQueue;
    VkCommandPool   m_vkCommandPool;
    VkCommandBuffer m_vkCommandBuffer;

};

} // namespace Renderer

} // namespace KompotEngine
