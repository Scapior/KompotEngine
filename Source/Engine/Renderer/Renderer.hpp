#pragma once

#include "global.hpp"
#include "Shader.hpp"
#include <vulkan/vulkan.hpp>
#include <boost/algorithm/string.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <set>
#include <array>

namespace KompotEngine
{

namespace Renderer
{

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription vertexInputBindingDescritpion = {};
        vertexInputBindingDescritpion.binding = 0_u32t;
        vertexInputBindingDescritpion.stride = sizeof(Vertex);
        vertexInputBindingDescritpion.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return vertexInputBindingDescritpion;
    }

    static std::array<VkVertexInputAttributeDescription, 2_u64t> getAttributeDescritptions()
    {
        std::array<VkVertexInputAttributeDescription, 2_u64t> vertexInputAttributeDescription;
        vertexInputAttributeDescription[0].binding = 0_u32t;
        vertexInputAttributeDescription[0].location = 0_u32t;
        vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
        vertexInputAttributeDescription[0].offset = offsetof(Vertex, position);

        vertexInputAttributeDescription[1].binding = 0_u32t;
        vertexInputAttributeDescription[1].location = 1_u32t;
        vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexInputAttributeDescription[1].offset = offsetof(Vertex, color);

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

    VkQueue          m_vkGraphicQueue;
    VkQueue          m_vkPresentQueue;

    VkFormat         m_vkImageFormat;

    VkSwapchainKHR   m_vkSwapchain;

    std::vector<VkImage>     m_vkImages;
    std::vector<VkImageView> m_vkImageViews;
    VkViewport m_vkViewport;
    VkRect2D   m_vkRect;

    std::vector<VkFramebuffer> m_vkFramebuffers;
    VkRenderPass    m_vkRenderPass;

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
};


} // namespace Renderer

} //namespace KompotEngine
