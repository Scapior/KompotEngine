#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

Renderer::Renderer(GLFWwindow *window, uint32_t width, uint32_t height)
    : m_glfwWindowHandler(window), m_screenWidth(width), m_screenHeight(height)
{
    glfwMakeContextCurrent(window);

    if (!glfwVulkanSupported())
    {
        // TODO:  error to log here
        exit(-1);
    }
    glfwSwapInterval(0);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}


void Renderer::run()
{
    while (!glfwWindowShouldClose(m_glfwWindowHandler))
    {
        glfwSwapBuffers(m_glfwWindowHandler);
        glfwPollEvents();
    }
}
