#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

Renderer::Renderer(GLFWwindow *window, uint32_t width, uint32_t height, const std::string &windowName)
    : m_glfwWindowHandler(window), m_screenWidth(width), m_screenHeight(height)
{
    createVkInstance(m_vkInstance, windowName);
    loadFuntions(m_vkInstance);
    setupDebugCallback(m_vkInstance, m_vkDebugMessenger);
    createSurface(m_vkInstance, m_glfwWindowHandler, m_vkSurface);
    createVulkanDevice(m_vkInstance, m_vkSurface, m_device);
    createSwapchain(m_device, m_vkSurface, width, height, m_swapchain);
    createRenderPass(m_device.device, m_swapchain, m_renderPass);
    createGraphicsPipeline(m_device.device, m_swapchain, m_renderPass, m_pipeline);
}

Renderer::~Renderer()
{
    vkDestroyRenderPass(m_device.device, m_renderPass, nullptr);
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
    pfn_vkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);    
}

void Renderer::run()
{
    while (!glfwWindowShouldClose(m_glfwWindowHandler)) // todo: remove all this
    {
        //glfwSwapBuffers(m_glfwWindowHandler);
        glfwPollEvents();
    }
}
