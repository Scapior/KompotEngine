#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>

namespace KompotEngine
{

namespace Renderer
{

class DescriptorPoolManager
{
public:
    DescriptorPoolManager(VkDevice);
    ~DescriptorPoolManager();

    operator VkDescriptorPool () const;

    std::vector<VkDescriptorSet> allocateDescriptorSet(uint32_t, const VkDescriptorSetLayout*);

private:
    VkDevice m_vkDevice;
    VkDescriptorPool m_vkDescriptorPool;

    static const auto MAX_POOL_SETS_SIZE = 2048_u32t;
};

} // namespace Renderer

} //namespace KompotEngine
