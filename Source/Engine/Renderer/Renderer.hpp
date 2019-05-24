#pragma once

#include "global.hpp"
#include "ResourcesMaker.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "../World.hpp"
#include "../IO/MeshesLoader.hpp"
#include "../IO/TgaLoader.hpp"
#include <vulkan/vulkan.hpp>
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
#include <memory>

namespace KompotEngine
{

namespace Renderer
{

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
    "VK_LAYER_LUNARG_standard_validation\0",
    "VK_LAYER_LUNARG_monitor"
#endif
};

class Renderer
{
public:
    Renderer(GLFWwindow*, const std::shared_ptr<World>&, const std::string&);
    void run();
    void resize();
    ~Renderer();
private:
    ResourcesMaker *m_resourcesMaker = nullptr;

    // window
    GLFWwindow *m_glfwWindowHandler = nullptr;
    std::shared_ptr<World> m_world;
    std::string m_windowsName;
    bool m_isResized;

    // width and height will be setted in createSwapchain -> chooseExtent
    VkExtent2D m_vkExtent;

    // vulkan members
    VkInstance       m_vkInstance = nullptr;
    VkSurfaceKHR     m_vkSurface = nullptr;

    VkPhysicalDevice m_vkPhysicalDevice  = nullptr; // will be implicitly destroyed with VkInstance
    VkDevice         m_vkDevice  = nullptr;

    VkQueue          m_vkGraphicsQueue;
    VkQueue          m_vkPresentQueue;

    VkFormat         m_vkImageFormat;

    VkSwapchainKHR   m_vkSwapchain;

    std::vector<std::shared_ptr<Image>> m_vkSwapchainImages;

    VkViewport m_vkViewport;
    VkRect2D   m_vkRect;

    std::vector<VkFramebuffer> m_vkFramebuffers;
    VkRenderPass    m_vkRenderPass;

    VkDescriptorSetLayout m_vkDescriptorSetLayout;    

    VkPipelineLayout m_vkPipelineLayout;
    VkPipeline       m_vkPipeline;

    VkCommandPool   m_vkCommandPool;

    std::vector<VkSemaphore> m_vkImageAvailableSemaphores;
    std::vector<VkSemaphore> m_vkRenderFinishedSemaphores;
    std::vector<VkFence>     m_vkInFlightFramesFence;

#ifdef ENGINE_DEBUG
    VkDebugUtilsMessengerEXT m_vkDebugMessenger;
    static PFN_vkCreateDebugUtilsMessengerEXT  pfn_vkCreateDebugUtilsMessengerEXT;
    static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT;
#endif

    std::shared_ptr<Image> m_vkDepthImage;

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
    void createGraphicsPipeline();
    void createCommandPool();
    void createSyncObjects();

    void cleanupSwapchain();
    void recreateSwapchain();

    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);

    void createDepthResources();
};


} // namespace Renderer

} //namespace KompotEngine
