#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

Renderer::Renderer(GLFWwindow *window, uint32_t width, uint32_t height, const std::string &windowName)
    : m_glfwWindowHandler(window), m_screenWidth(width), m_screenHeight(height), m_vkInstance(nullptr)
{
    createVkInstance(m_vkInstance, windowName);
    loadFuntions(m_vkInstance);
    setupDebugCallback(m_vkInstance, m_vkDebugMessenger);
}

Renderer::~Renderer()
{
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
