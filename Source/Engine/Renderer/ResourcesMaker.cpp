#include "ResourcesMaker.hpp"

using namespace KompotEngine::Renderer;

ResourcesMaker::ResourcesMaker(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkCommandPool vkCommandPool, VkQueue vkGraphicsQueue)
    : m_vkDevice(vkDevice), m_vkPhysicalDevice(vkPhysicalDevice), m_vkCommandPool(vkCommandPool), m_vkGraphicsQueue(vkGraphicsQueue) {}

SingleTimeCommandBuffer ResourcesMaker::createSingleTimeCommandBuffer() const
{
    return SingleTimeCommandBuffer(m_vkDevice, m_vkCommandPool, m_vkGraphicsQueue);
}

uint32_t ResourcesMaker::findMemoryType(uint32_t requiredTypes, VkMemoryPropertyFlags requiredProperties) const
{
    VkPhysicalDeviceMemoryProperties vkPhysicalDevicememoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &vkPhysicalDevicememoryProperties);

    for (auto i = 0_u32t; i < vkPhysicalDevicememoryProperties.memoryTypeCount; ++i)
    {
        if ((requiredTypes & (1 << i)) && (vkPhysicalDevicememoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)
        {
            return i;
        }
    }

    Log::getInstance() << "ResourcesMaker::findMemoryType(): Failed to find suitable memory type. Terminated."<< std::endl;
    std::terminate();
}

std::shared_ptr<Model> ResourcesMaker::createModelFromFile(const fs::path &path)
{
    m_modelsLoader.loadFile(path);
    auto model = m_modelsLoader.generateModel();
    if (model == nullptr)
    {
        return {};
    }

    VkDeviceSize vkVerticesBufferSize = model->getVerticiesSizeForBuffer();
    auto verticesStagingBuffer = createBuffer(
                             vkVerticesBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkResult resultCode = verticesStagingBuffer->copyFromRawPointer(model->getVerticesData(), vkVerticesBufferSize);
    if (resultCode != VK_SUCCESS)
    {
        return {};
    }

    auto vkVerticesBuffer = createBufferCopy(verticesStagingBuffer,
                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceSize vkIndecesBufferSize = model->getVerticiesIndecesSizeForBuffer();
    auto indecesStagingBuffer = createBuffer(
                             vkIndecesBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    resultCode = indecesStagingBuffer->copyFromRawPointer(model->getVerticiesIndicesData(), vkIndecesBufferSize);
    if (resultCode != VK_SUCCESS)
    {
        return {};
    }

    auto vkIndecesBuffer = createBufferCopy(
                             indecesStagingBuffer,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    model->setBuffer(vkVerticesBuffer, vkIndecesBuffer);
    return model;
}

std::shared_ptr<Buffer> ResourcesMaker::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperies) const
{
    auto vkBuffer = createVkBuffer(size, usage);
    if (vkBuffer == nullptr)
    {
        return {};
    }

    auto vkBufferMemory = allocateAndBindVkBufferMemory(vkBuffer, memoryProperies);
    if (vkBufferMemory == nullptr)
    {
        return {};
    }

    return std::make_shared<Buffer>(m_vkDevice, vkBuffer, vkBufferMemory, size);
}

std::shared_ptr<Buffer> ResourcesMaker::createBufferCopy(const std::shared_ptr<Buffer> &sourceBuffer, VkBufferUsageFlags usage,
                                                         VkMemoryPropertyFlags memoryProperies) const
{
    const auto bufferSize = sourceBuffer->getSize();

    auto vkBuffer = createVkBuffer(bufferSize, usage);
    if (vkBuffer == nullptr)
    {
        return {};
    }

    auto  vkBufferMemory = allocateAndBindVkBufferMemory(vkBuffer, memoryProperies);
    if (vkBufferMemory == nullptr)
    {
        return {};
    }

    auto commandBuffer = createSingleTimeCommandBuffer();
    VkBufferCopy vkBufferCopyRegion = {};
    vkBufferCopyRegion.size = bufferSize;

    vkCmdCopyBuffer(commandBuffer, sourceBuffer->getBuffer(), vkBuffer, 1_u32t, &vkBufferCopyRegion);
    commandBuffer.submit();

    return std::make_shared<Buffer>(m_vkDevice, vkBuffer, vkBufferMemory, bufferSize);
}

VkBuffer ResourcesMaker::createVkBuffer(VkDeviceSize size, VkBufferUsageFlags usage) const
{
    VkBuffer vkBuffer = nullptr;
    VkBufferCreateInfo vkBufferCreateInfo = {};
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.size = size;
    vkBufferCreateInfo.usage = usage;
    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto resultCode = vkCreateBuffer(m_vkDevice, &vkBufferCreateInfo, nullptr, &vkBuffer);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createBuffer(): Function vkCreateBuffer call failed with a code" << resultCode << ". Terminated."<< std::endl;
    }
    return vkBuffer;
}

VkDeviceMemory ResourcesMaker::allocateAndBindVkBufferMemory(VkBuffer vkBuffer, VkMemoryPropertyFlags memoryProperies) const
{
    VkDeviceMemory vkBufferMemory = nullptr;

    VkMemoryRequirements vkMemoryRequirements = {};
    vkGetBufferMemoryRequirements(m_vkDevice, vkBuffer, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMomoryAllocateInfo = {};
    vkMomoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMomoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMomoryAllocateInfo.memoryTypeIndex = findMemoryType(vkMemoryRequirements.memoryTypeBits, memoryProperies);

    auto resultCode = vkAllocateMemory(m_vkDevice, &vkMomoryAllocateInfo, nullptr, &vkBufferMemory);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createBuffer(): Function vkAllocateMemory call failed with a code" << resultCode << ". Terminated."<< std::endl;
        return nullptr;
    }

    resultCode = vkBindBufferMemory(m_vkDevice, vkBuffer, vkBufferMemory, 0_u32t);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createBuffer(): Function vkBindBufferMemory call failed with a code" << resultCode << ". Terminated."<< std::endl;
        vkFreeMemory(m_vkDevice, vkBufferMemory, nullptr);
        vkBufferMemory = nullptr;
    }
    return vkBufferMemory;
}
