#include "Engine.hpp"

using namespace KompotEngine;

Engine::Engine(const std::string& name, const EngineConfig& config)
    : m_instanceName(name), m_engineSettings(config)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    const int width = static_cast<int>(m_engineSettings.windowWidth);
    const int height = static_cast<int>(m_engineSettings.windowHeight);
    m_glfwWindowHandler = glfwCreateWindow(width, height, m_instanceName.c_str(), nullptr, nullptr);
    m_renderer = new Renderer::Renderer(m_glfwWindowHandler, m_engineSettings.windowWidth, m_engineSettings.windowHeight);
}

Engine::~Engine()
{
    glfwDestroyWindow(m_glfwWindowHandler);
}

void Engine::run()
{
    std::thread(&Renderer::Renderer::run, m_renderer).detach();
    while (!glfwWindowShouldClose(m_glfwWindowHandler)) // TODO: replace this
    {
    }
}
