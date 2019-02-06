#pragma once

#include "global.hpp"
#include "VulkanUtils.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <limits>

namespace KompotEngine
{

namespace Renderer
{

static const uint64_t MAX_FRAMES_IN_FLIGHT = 2_u64t;
class Renderer
{
public:
    Renderer(GLFWwindow*, uint32_t, uint32_t, const std::string&);
    void run();
    ~Renderer();
private:
    // window
    GLFWwindow *m_glfwWindowHandler;
    uint32_t    m_screenWidth;
    uint32_t    m_screenHeight;

    // vulkan members
    VkInstance       m_vkInstance;
    VkSurfaceKHR     m_vkSurface;

    VulkanDevice    m_device;
    VulkanSwapchain m_swapchain;
    VkRenderPass    m_renderPass;
    VulkanPipeline  m_pipeline;

    VkCommandPool   m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers; // automatically freed when their command pool is destroyed

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence>     m_inFlightFramesFence;

    VkDebugUtilsMessengerEXT m_vkDebugMessenger;

};


} // namespace Renderer

} //namespace KompotEngine
