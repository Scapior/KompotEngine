#include "DescriptorPoolManager.hpp"

using namespace KompotEngine::Renderer;

DescriptorPoolManager::DescriptorPoolManager(VkDevice vkDevice)
    : m_vkDevice(vkDevice)
{
    std::array<VkDescriptorPoolSize, 2_u32t> vkDescriptorPoolsSizes = {};

    vkDescriptorPoolsSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolsSizes[0].descriptorCount = MAX_POOL_SETS_SIZE;

    vkDescriptorPoolsSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolsSizes[1].descriptorCount = MAX_POOL_SETS_SIZE;

    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo = {};
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.poolSizeCount = vkDescriptorPoolsSizes.size();
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolsSizes.data();
    vkDescriptorPoolCreateInfo.maxSets = MAX_POOL_SETS_SIZE;

    const auto resultCode = vkCreateDescriptorPool(m_vkDevice, &vkDescriptorPoolCreateInfo, nullptr, &m_vkDescriptorPool);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "DescriptorPoolManager::DescriptorPoolManager(): Function vkCreateDescriptorPool call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

DescriptorPoolManager::~DescriptorPoolManager()
{
    vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);
}

KompotEngine::Renderer::DescriptorPoolManager::operator VkDescriptorPool() const
{
    return m_vkDescriptorPool;
}

std::vector<VkDescriptorSet> DescriptorPoolManager::allocateDescriptorSet(uint32_t count, const VkDescriptorSetLayout *vkDescriptorSetLayout)
{
    std::vector<VkDescriptorSetLayout> vkDescriptorSetsLayouts(static_cast<std::size_t>(count), *vkDescriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_vkDescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = count;
    descriptorSetAllocateInfo.pSetLayouts = vkDescriptorSetsLayouts.data();

    std::vector<VkDescriptorSet> vkDescriptorSets{count};

    const auto resultCode = vkAllocateDescriptorSets(m_vkDevice, &descriptorSetAllocateInfo, vkDescriptorSets.data());
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "DescriptorPoolManager::allocateDescriptorSet(): Function vkAllocateDescriptorSets call failed with code " << resultCode << "." << std::endl;
    }
    return vkDescriptorSets;
}
