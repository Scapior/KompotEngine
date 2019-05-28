#pragma once

#include "global.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "SingleTimeCommandBuffer.hpp"
#include "../IO/MeshesLoader.hpp"
#include "../IO/TgaLoader.hpp"
#include "../MeshObject.hpp"
#include "DescriptorPoolManager.hpp"
#include <vulkan/vulkan.hpp>
#include <memory>
#include <array>
#include <filesystem>

namespace fs = std::filesystem;

namespace KompotEngine
{

namespace Renderer
{

class ResourcesMaker
{
public:
    ResourcesMaker(VkPhysicalDevice, VkDevice, VkCommandPool, VkQueue, VkDescriptorSetLayout);

    SingleTimeCommandBuffer createSingleTimeCommandBuffer() const;

    std::shared_ptr<MeshObject> createMeshObject(const std::string&, std::shared_ptr<Mesh>, std::shared_ptr<Image>);
    std::shared_ptr<Mesh> createMeshFromFile(const fs::path&) const;
    std::shared_ptr<Image> createTextureFromFile(const fs::path&) const;
    std::shared_ptr<Image> createSwapchainImage(VkImage, VkFormat) const;
    std::shared_ptr<Image> createImage(VkExtent2D, uint32_t, VkFormat, VkImageLayout, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImageAspectFlags) const;

    std::shared_ptr<Buffer> createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags) const;
    std::shared_ptr<Buffer> createBufferCopy(const std::shared_ptr<Buffer>&, VkBufferUsageFlags, VkMemoryPropertyFlags) const;

private:
    VkDevice         m_vkDevice;
    VkPhysicalDevice m_vkPhysicalDevice;
    VkCommandPool    m_vkCommandPool;
    VkQueue          m_vkGraphicsQueue;
    mutable DescriptorPoolManager m_descriptorPoolManager;
    VkDescriptorSetLayout m_vkDescriptorSetLayout;

    mutable IO::ModelsLoader      m_ModelsLoader;
    mutable IO::TgaLoader         m_texturesLoader;

    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags) const;
    VkBuffer createVkBuffer(VkDeviceSize, VkBufferUsageFlags) const;
    VkDeviceMemory allocateAndBindVkBufferMemory(VkBuffer, VkMemoryPropertyFlags) const;
    VkResult copyBufferToImage(Buffer&, Image&) const;
};


} // namespace Renderer

} // namespace KompotEngine


