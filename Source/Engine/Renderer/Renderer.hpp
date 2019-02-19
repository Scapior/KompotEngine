#pragma once

#include "global.hpp"
#include "Shader.hpp"
#include <vulkan/vulkan.hpp>
#include <boost/algorithm/string.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <set>
#include <array>
#include <chrono>

namespace KompotEngine
{

namespace Renderer
{

struct UnifromBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 textureCoordinates;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription vertexInputBindingDescritpion = {};
        vertexInputBindingDescritpion.binding = 0_u32t;
        vertexInputBindingDescritpion.stride = sizeof(Vertex);
        vertexInputBindingDescritpion.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return vertexInputBindingDescritpion;
    }

    static std::array<VkVertexInputAttributeDescription, 3_u64t> getAttributeDescritptions()
    {
        std::array<VkVertexInputAttributeDescription, 3_u64t> vertexInputAttributeDescription;
        vertexInputAttributeDescription[0].binding = 0_u32t;
        vertexInputAttributeDescription[0].location = 0_u32t;
        vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexInputAttributeDescription[0].offset = offsetof(Vertex, position);

        vertexInputAttributeDescription[1].binding = 0_u32t;
        vertexInputAttributeDescription[1].location = 1_u32t;
        vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexInputAttributeDescription[1].offset = offsetof(Vertex, color);

        vertexInputAttributeDescription[2].binding = 0_u32t;
        vertexInputAttributeDescription[2].location = 2_u32t;
        vertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
        vertexInputAttributeDescription[2].offset = offsetof(Vertex, textureCoordinates);

        return vertexInputAttributeDescription;
    }
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR       format;
    VkPresentModeKHR         presentMode;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;

    bool isComplete() const
    {
        return graphicFamilyIndex.has_value() &&
               presentFamilyIndex.has_value();
    }

};

static const uint64_t MAX_FRAMES_IN_FLIGHT = 2_u64t;

static std::vector<const char*> validationLayers {
#ifdef ENGINE_DEBUG
    "VK_LAYER_LUNARG_standard_validation"
#endif
};

class Renderer
{
public:
    Renderer(GLFWwindow*, const std::string&);
    void run();
    void resize();
    ~Renderer();
private:
    // window
    GLFWwindow *m_glfwWindowHandler;
    std::string m_windowsName;
    bool m_isResized;

    // width and height will be setted in createSwapchain -> chooseExtent
    uint32_t    m_width;
    uint32_t    m_height;

    // vulkan members
    VkInstance       m_vkInstance;
    VkSurfaceKHR     m_vkSurface;

    VkPhysicalDevice m_vkPhysicalDevice; // will be implicitly destroyed with VkInstance
    VkDevice         m_vkDevice;

    VkQueue          m_vkGraphicsQueue;
    VkQueue          m_vkPresentQueue;

    VkFormat         m_vkImageFormat;

    VkSwapchainKHR   m_vkSwapchain;

    std::vector<VkImage>     m_vkImages;
    std::vector<VkImageView> m_vkImageViews;
    VkViewport m_vkViewport;
    VkRect2D   m_vkRect;

    std::vector<VkFramebuffer> m_vkFramebuffers;
    VkRenderPass    m_vkRenderPass;

    VkDescriptorSetLayout m_vkDescriptorSetLayout;
    VkDescriptorPool m_vkDescriptorPool;
    std::vector<VkDescriptorSet> m_vkDescriptorSets;
    VkPipelineLayout m_vkPipelineLayout;
    VkPipeline       m_vkPipeline;

    VkCommandPool   m_vkCommandPool;
    std::vector<VkCommandBuffer> m_vkCommandBuffers; // automatically freed when their command pool is destroyed

    std::vector<VkSemaphore> m_vkImageAvailableSemaphores;
    std::vector<VkSemaphore> m_vkRenderFinishedSemaphores;
    std::vector<VkFence>     m_vkInFlightFramesFence;

#ifdef ENGINE_DEBUG
    VkDebugUtilsMessengerEXT m_vkDebugMessenger;
    static PFN_vkCreateDebugUtilsMessengerEXT  pfn_vkCreateDebugUtilsMessengerEXT;
    static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT;
#endif

    VkBuffer m_vkVertexBuffer;
    VkDeviceMemory m_vkVertexBufferMemory;
    VkBuffer m_vkIndexBuffer;
    VkDeviceMemory m_vkIndexBufferMemory;

    std::vector<VkBuffer>       m_vkUniformMatricesBuffers;
    std::vector<VkDeviceMemory> m_vkUniformMatricesBuffersMemory;

    VkImage m_vkTextureImage;
    VkImageView m_vkTextureImageView;
    VkDeviceMemory m_vkTextureImageMemory;
    VkSampler m_vkTextureSampler;

    VkImage m_vkDepthImage;
    VkDeviceMemory m_vkDepthImageMemory;
    VkImageView m_vkDepthImageView;

    void createVkInstance();
    void setupDebugCallback();
    void createSurface();
    void selectPysicalDevice();
    QueueFamilyIndices findQueueFamilies();
    SwapchainSupportDetails getSwapchainDetails();
    void createDevice();
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR&);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>&);
    void createSwapchain();
    void createRenderPass();
    void createFramebuffers();
    void createDescriptorSetLayout();
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSets();
    void createGraphicsPipeline();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    void cleanupSwapchain();
    void recreateSwapchain();

    void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
    void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
    void createVertexBuffer();
    void createIndexBuffer();
    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);
    void updateUniformBuffer(uint32_t);
    void createTextureImage();
    void createTextureImageView();
    void createImage(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer);
    void copyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t);
    void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout);
    void createImageView(VkImage, VkFormat, VkImageAspectFlags, VkImageView&);
    void createTextureSampler();

    void createDepthResources();
};


} // namespace Renderer

} //namespace KompotEngine
