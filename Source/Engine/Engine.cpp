#include "Engine.hpp"

using namespace KompotEngine;

Engine::Engine(const std::string& name, const EngineConfig& config)
    : m_instanceName(name), m_engineSettings(config)
{
    glfwInit();
    glfwSwapInterval(0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    if (m_engineSettings.gapi == Renderer::GAPI::Vulkan)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
}

Engine::~Engine()
{
    glfwDestroyWindow(m_glfwWindowHandler);
}

void Engine::run()
{
    const int width = static_cast<int>(m_engineSettings.windowWidth);
    const int height = static_cast<int>(m_engineSettings.windowHeight);
    m_glfwWindowHandler = glfwCreateWindow(width, height, m_instanceName.c_str(), nullptr, nullptr);
    while (!glfwWindowShouldClose(m_glfwWindowHandler))
    {
        glfwSwapBuffers(m_glfwWindowHandler);
        glfwPollEvents();
    }
}
