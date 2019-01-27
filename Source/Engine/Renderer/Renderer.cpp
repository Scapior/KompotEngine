#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

Renderer::Renderer(GLFWwindow *window, uint32_t width, uint32_t height, const std::string &windowName)
    : m_glfwWindowHandler(window), m_screenWidth(width), m_screenHeight(height)
{
    createVkInstance(m_vkInstance, windowName);
    loadFuntions(m_vkInstance);
    setupDebugCallback(m_vkInstance, m_vkDebugMessenger);
    selectPhysicalDevice(m_vkInstance, m_vkPhysicalDevice);
    createSurface(m_vkInstance, m_glfwWindowHandler, m_vkSurface);
    createLogicalDeviceAndQueue(m_vkPhysicalDevice, m_vkDevice, m_vkGraphicQueue, m_vkPresentQueue, m_vkSurface);
    createSwapchain(m_vkDevice, m_vkPhysicalDevice, m_vkSurface, width, height, m_vkSwapchain);
}

Renderer::~Renderer()
{
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, nullptr);
    vkDestroyDevice(m_vkDevice, nullptr);
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
