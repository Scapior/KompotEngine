#include "ResourcesMaker.hpp"

using namespace KompotEngine::Renderer;

ResourcesMaker::ResourcesMaker(Device &device,
                               VkCommandPool vkCommandPool,
                               VkDescriptorSetLayout vkDescriptorSetLayout)
    : m_device(device),
      m_vkCommandPool(vkCommandPool),
      m_descriptorPoolManager(device),
      m_vkDescriptorSetLayout(vkDescriptorSetLayout)
{
}

SingleTimeCommandBuffer ResourcesMaker::createSingleTimeCommandBuffer() const
{
    return SingleTimeCommandBuffer(m_device, m_vkCommandPool, m_device.getGraphicsQueue());
}

std::shared_ptr<KompotEngine::MeshObject> ResourcesMaker::createMeshObject(
        const std::string &className,
        std::shared_ptr<Mesh> meshPointer,
        std::shared_ptr<Image> texturePointer)
{
    std::string modelFileName;
    std::string textureFileName;

    OptionsParser classOptions;
    classOptions.addOptions()
        ("ModelFileName", "", std::string(), &modelFileName)
        ("TextureName",   "", std::string(), &textureFileName);

    std::ifstream configFile(className + ".kec");
    if (configFile.is_open())
    {
        classOptions.loadFromFile(configFile);
    }
    classOptions.notify();

    if (!meshPointer)
    {
        meshPointer = createMeshFromFile(modelFileName);
    }

    if (!texturePointer)
    {
        texturePointer = createTextureFromFile(textureFileName);
    }


    VkDeviceSize vkUniformSize = sizeof(UnifromBufferObject);

    std::vector<std::shared_ptr<Buffer>> vkUniformMatricesBuffers{2};
    for (auto i = 0_u64t; i < vkUniformMatricesBuffers.size(); ++i)
    {
        vkUniformMatricesBuffers[i] = createBuffer(
                     vkUniformSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
    }

    auto vkDescriptorSets = m_descriptorPoolManager.allocateDescriptorSet(2, &m_vkDescriptorSetLayout);
    for (auto i = 0_u64t; i < vkDescriptorSets.size(); ++i)
    {
        VkDescriptorBufferInfo vkDescriptorBufferInfo = {};
        vkDescriptorBufferInfo.buffer = vkUniformMatricesBuffers[i]->getBuffer();
        vkDescriptorBufferInfo.offset = 0_64t;
        vkDescriptorBufferInfo.range = sizeof(UnifromBufferObject);

        VkDescriptorImageInfo vkDescriptorImageInfo = {};
        vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkDescriptorImageInfo.imageView = texturePointer->getImageView();
        vkDescriptorImageInfo.sampler = texturePointer->getSampler();

        std::array<VkWriteDescriptorSet, 2_u32t> vkWriteDescriptorsets = {};

        vkWriteDescriptorsets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorsets[0].dstSet = vkDescriptorSets[i];
        vkWriteDescriptorsets[0].dstBinding = 0_u32t;
        vkWriteDescriptorsets[0].dstArrayElement = 0_u32t;
        vkWriteDescriptorsets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vkWriteDescriptorsets[0].descriptorCount = 1_u32t;
        vkWriteDescriptorsets[0].pBufferInfo = &vkDescriptorBufferInfo;

        vkWriteDescriptorsets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorsets[1].dstSet = vkDescriptorSets[i];
        vkWriteDescriptorsets[1].dstBinding = 1_u32t;
        vkWriteDescriptorsets[1].dstArrayElement = 0_u32t;
        vkWriteDescriptorsets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        vkWriteDescriptorsets[1].descriptorCount = 1_u32t;
        vkWriteDescriptorsets[1].pImageInfo = &vkDescriptorImageInfo;

        vkUpdateDescriptorSets(m_device, vkWriteDescriptorsets.size(), vkWriteDescriptorsets.data(), 0_u32t, nullptr);
    }

    return std::make_shared<KompotEngine::MeshObject>(m_device,
                                                      m_descriptorPoolManager,
                                                      className,
                                                      vkDescriptorSets,
                                                      meshPointer,
                                                      texturePointer,
                                                      vkUniformMatricesBuffers);
}

uint32_t ResourcesMaker::findMemoryType(uint32_t requiredTypes, VkMemoryPropertyFlags requiredProperties) const
{
    VkPhysicalDeviceMemoryProperties vkPhysicalDevicememoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device, &vkPhysicalDevicememoryProperties);

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

std::shared_ptr<Mesh> ResourcesMaker::createMeshFromFile(const fs::path &path) const
{
    m_ModelsLoader.loadFile(path);
    auto mesh = m_ModelsLoader.generateModel();
    if (mesh == nullptr)
    {
        return {};
    }

    VkDeviceSize vkVerticesBufferSize = mesh->getVerticiesSizeForBuffer();
    auto verticesStagingBuffer = createBuffer(
                             vkVerticesBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkResult resultCode = verticesStagingBuffer->copyFromRawPointer(mesh->getVerticesData(), vkVerticesBufferSize);
    if (resultCode != VK_SUCCESS)
    {
        return {};
    }

    auto vkVerticesBuffer = createBufferCopy(verticesStagingBuffer,
                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceSize vkIndecesBufferSize = mesh->getVerticiesIndecesSizeForBuffer();
    auto indecesStagingBuffer = createBuffer(
                             vkIndecesBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    resultCode = indecesStagingBuffer->copyFromRawPointer(mesh->getVerticiesIndicesData(), vkIndecesBufferSize);
    if (resultCode != VK_SUCCESS)
    {
        return {};
    }

    auto vkIndecesBuffer = createBufferCopy(
                             indecesStagingBuffer,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mesh->setBuffer(vkVerticesBuffer, vkIndecesBuffer);

    return mesh;
}

std::shared_ptr<Image> ResourcesMaker::createTextureFromFile(const fs::path &filePath) const
{
    m_texturesLoader.loadFile(filePath);
    auto textureBytes = m_texturesLoader.getLastLoadedTextureBytes();
    if (textureBytes.empty())
    {
        return {};
    }
    const auto textureExtent = m_texturesLoader.getLastLoadedTextureExtent();

    auto textureImageMipLevelsCount = 1_u32t + static_cast<uint32_t>
                    (std::floor(std::log2(std::max(textureExtent.width, textureExtent.height))));

    auto stagingBuffer = createBuffer(
                             textureBytes.size(),
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer->copyFromRawPointer(textureBytes.data(), textureBytes.size());

    auto image = createImage(
                     textureExtent,
                     textureImageMipLevelsCount,
                     VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_TILING_OPTIMAL,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     VK_IMAGE_ASPECT_COLOR_BIT);


    image->transitImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, createSingleTimeCommandBuffer());
    copyBufferToImage(*stagingBuffer.get(), *image.get());
    auto commandBuffer = createSingleTimeCommandBuffer();
    image->generateMipLevels(commandBuffer);
    commandBuffer.submit();

    return image;
}

std::shared_ptr<Image> ResourcesMaker::createSwapchainImage(VkImage vkImage, VkFormat vkFormat) const
{
    VkImageViewCreateInfo vkImageViewCreateInfo = {};
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.image = vkImage;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format = vkFormat;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0_u32t;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1_u32t;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0_u32t;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1_u32t;

    VkImageView vkImageView{};
    auto resultCode = vkCreateImageView(m_device, &vkImageViewCreateInfo, nullptr, &vkImageView);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createSwapchainImage(): Function vkCreateImageView call failed with a code " << resultCode << "." << std::endl;
        std::terminate();
    }
    return std::make_shared<Image>(m_device, vkImage, vkImageView);
}

std::shared_ptr<Image> ResourcesMaker::createImage(
        VkExtent2D extent,
        uint32_t mipLevelsCount,
        VkFormat format,
        VkImageLayout initialLayout,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImageAspectFlags imageAspectFlags) const
{  
    VkImage vkImage{};

    VkImageCreateInfo vkImageCreateInfo = {};
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = format;
    vkImageCreateInfo.extent.width = extent.width;
    vkImageCreateInfo.extent.height = extent.height;
    vkImageCreateInfo.extent.depth = 1_u32t;
    vkImageCreateInfo.mipLevels = mipLevelsCount;
    vkImageCreateInfo.arrayLayers = 1_u32t;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = tiling;
    vkImageCreateInfo.usage = usage;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = initialLayout;

    auto resultCode = vkCreateImage(m_device, &vkImageCreateInfo, nullptr, &vkImage);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createImage(): Function vkCreateImage call failed with a code " << resultCode << "." << std::endl;
        return {};
    }

    VkMemoryRequirements vkMemoryRequirements;
    vkGetImageMemoryRequirements(m_device, vkImage, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMemoryAllocateInfo = {};
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = findMemoryType(vkMemoryRequirements.memoryTypeBits, properties);

    VkDeviceMemory vkImageMemory;
    resultCode = vkAllocateMemory(m_device, &vkMemoryAllocateInfo, nullptr, &vkImageMemory);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createImage(): Function vkAllocateMemory call failed with a code " << resultCode << "." << std::endl;
        vkDestroyImage(m_device, vkImage, nullptr);
        return {};
    }

    resultCode = vkBindImageMemory(m_device, vkImage, vkImageMemory, 0_u64t);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createImage(): Function vkBindImageMemory call failed with a code " << resultCode << "." << std::endl;
        vkDestroyImage(m_device, vkImage, nullptr);
        vkFreeMemory(m_device, vkImageMemory, nullptr);
        return {};
    }

    VkImageViewCreateInfo vkImageViewCreateInfo = {};
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.image = vkImage;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format = format;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0_u32t;
    vkImageViewCreateInfo.subresourceRange.levelCount = mipLevelsCount;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0_u32t;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1_u32t;

    VkImageView vkImageView{};
    resultCode = vkCreateImageView(m_device, &vkImageViewCreateInfo, nullptr, &vkImageView);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createImage(): Function vkCreateImageView call failed with a code " << resultCode << "." << std::endl;
        vkDestroyImage(m_device, vkImage, nullptr);
        vkFreeMemory(m_device, vkImageMemory, nullptr);
        return {};
    }

    VkSampler vkImageSampler{};
    if (mipLevelsCount > 1 && imageAspectFlags & VK_IMAGE_ASPECT_COLOR_BIT)
    {
        VkSamplerCreateInfo vkSamplerCreateInfo = {};
        vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        vkSamplerCreateInfo.mipLodBias = 0.0f;
        vkSamplerCreateInfo.minLod = 0.0f;
        vkSamplerCreateInfo.maxLod = static_cast<float>(mipLevelsCount);
        vkSamplerCreateInfo.anisotropyEnable = VK_TRUE;
        vkSamplerCreateInfo.maxAnisotropy = 16_u32t;
        vkSamplerCreateInfo.compareEnable = VK_FALSE;
        vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

        resultCode = vkCreateSampler(m_device, &vkSamplerCreateInfo, nullptr, &vkImageSampler);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "ResourcesMaker::createImage(): Function vkCreateSampler call failed with a code " << resultCode << "." << std::endl;
            vkDestroyImage(m_device, vkImage, nullptr);
            vkDestroyImageView(m_device, vkImageView, nullptr);
            vkFreeMemory(m_device, vkImageMemory, nullptr);
            return {};
        }
    }
    auto image = std::make_shared<Image>(m_device, extent, vkImage, vkImageView, vkImageSampler, format,
                                         initialLayout, vkImageMemory, imageAspectFlags, mipLevelsCount);
    if (vkImageSampler)
    {
        image->transitImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, createSingleTimeCommandBuffer());
    }
    return image;
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

    return std::make_shared<Buffer>(m_device, vkBuffer, vkBufferMemory, size);
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

    return std::make_shared<Buffer>(m_device, vkBuffer, vkBufferMemory, bufferSize);
}

VkBuffer ResourcesMaker::createVkBuffer(VkDeviceSize size, VkBufferUsageFlags usage) const
{
    VkBuffer vkBuffer = nullptr;
    VkBufferCreateInfo vkBufferCreateInfo = {};
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.size = size;
    vkBufferCreateInfo.usage = usage;
    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto resultCode = vkCreateBuffer(m_device, &vkBufferCreateInfo, nullptr, &vkBuffer);
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
    vkGetBufferMemoryRequirements(m_device, vkBuffer, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMomoryAllocateInfo = {};
    vkMomoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMomoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMomoryAllocateInfo.memoryTypeIndex = findMemoryType(vkMemoryRequirements.memoryTypeBits, memoryProperies);

    auto resultCode = vkAllocateMemory(m_device, &vkMomoryAllocateInfo, nullptr, &vkBufferMemory);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createBuffer(): Function vkAllocateMemory call failed with a code" << resultCode << ". Terminated."<< std::endl;
        return nullptr;
    }

    resultCode = vkBindBufferMemory(m_device, vkBuffer, vkBufferMemory, 0_u32t);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "ResourcesMaker::createBuffer(): Function vkBindBufferMemory call failed with a code" << resultCode << ". Terminated."<< std::endl;
        vkFreeMemory(m_device, vkBufferMemory, nullptr);
        vkBufferMemory = nullptr;
    }
    return vkBufferMemory;
}

VkResult ResourcesMaker::copyBufferToImage(Buffer &buffer, Image &image) const
{
    auto commandBuffer = createSingleTimeCommandBuffer();
    const auto imageExtent = image.getExtent();

    VkBufferImageCopy vkBufferImageCopyRegion = {};
    vkBufferImageCopyRegion.bufferOffset = 0_u32t;
    vkBufferImageCopyRegion.bufferRowLength = 0_u32t;
    vkBufferImageCopyRegion.bufferImageHeight = 0_u32t;
    vkBufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopyRegion.imageSubresource.mipLevel = 0_u32t;
    vkBufferImageCopyRegion.imageSubresource.baseArrayLayer = 0_u32t;
    vkBufferImageCopyRegion.imageSubresource.layerCount = 1_u32t;
    vkBufferImageCopyRegion.imageOffset = {0_32t, 0_32t, 0_32t};
    vkBufferImageCopyRegion.imageExtent.width = imageExtent.width;
    vkBufferImageCopyRegion.imageExtent.height = imageExtent.height;
    vkBufferImageCopyRegion.imageExtent.depth = 1_u32t;

    vkCmdCopyBufferToImage(commandBuffer, buffer.getBuffer(), image.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1_u32t, &vkBufferImageCopyRegion);
    commandBuffer.submit();
    return VK_SUCCESS;
}
