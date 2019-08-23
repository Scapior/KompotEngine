#include "Engine.hpp"

using namespace KompotEngine;

Engine::Engine(const std::string& name, const EngineConfig& config)
    : m_instanceName(name), m_engineSettings(config)
{
    auto &log = Log::getInstance();
    const int width = static_cast<int>(m_engineSettings.windowWidth);
    const int height = static_cast<int>(m_engineSettings.windowHeight);

	//glfwSetWindowTitle(m_glfwWindowHandler, m_instanceName.c_str());

    m_world.reset(new World());
    //m_pythonModule = new PythonModule(m_world);
    //m_renderer = new Renderer::Renderer(m_glfwWindowHandler, m_instanceName);

    log << "Renderer successfully initialized." << std::endl;
}

Engine::~Engine()
{

}

void Engine::run()
{
    //std::thread(&Renderer::Renderer::run, m_renderer, m_world).detach();
}
