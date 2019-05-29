#pragma once

#include "global.hpp"
#include "Device.hpp"
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

static const uint64_t MAX_FRAMES_IN_FLIGHT = 2_u64t;

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
    Device m_device;
    VkSurfaceKHR m_vkSurface;

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

    std::shared_ptr<Image> m_vkDepthImage;

    void createSurface(); 
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

    void createDepthResources();
};


} // namespace Renderer

} //namespace KompotEngine
