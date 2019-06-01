#include "Engine.hpp"

using namespace KompotEngine;

Engine::Engine(const std::string& name, const EngineConfig& config)
    : m_instanceName(name), m_engineSettings(config)
{
    auto &log = Log::getInstance();
    log << "GLFW init code: " <<  glfwInit() << std::endl;
    glfwSetErrorCallback(Log::callbackForGlfw);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //no GL context
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    const int width = static_cast<int>(m_engineSettings.windowWidth);
    const int height = static_cast<int>(m_engineSettings.windowHeight);
    m_glfwWindowHandler = glfwCreateWindow(width, height, m_instanceName.c_str(), nullptr, nullptr);
    if (m_glfwWindowHandler == nullptr)
    {
        log << "glfwCreateWindow error: " <<  glfwInit() << std::endl;
        std::terminate();
    }
    else
    {
        log << "Created GLFW window \"" <<  m_instanceName << "\"." << std::endl;
    }
    glfwSetWindowUserPointer(m_glfwWindowHandler, this);
    //glfwSetInputMode(m_glfwWindowHandler, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    if (m_engineSettings.isMaximized)
    {
        glfwMaximizeWindow(m_glfwWindowHandler); // not work
    }
    if (!glfwVulkanSupported())
    {
        log << "Vulkan not supported! Terminated." << std::endl;
        std::terminate();
    }

#ifdef ENGINE_DEBUG
    std::stringstream newWindowTitile;
    newWindowTitile << m_instanceName
                    << " - build " << std::to_string(BUILD_VERSION)
                    << std::hex << " [" << GIT_HASH_SHORT << "]" << std::dec;
    glfwSetWindowTitle(m_glfwWindowHandler, newWindowTitile.str().c_str());
#endif

    m_world.reset(new World());
    m_pythonModule = new PythonModule(m_world);
    m_renderer = new Renderer::Renderer(m_glfwWindowHandler, m_world, m_instanceName);

    log << "Renderer successfully initialized." << std::endl;

    glfwSetFramebufferSizeCallback(m_glfwWindowHandler, resizeCallback);
    glfwSetKeyCallback(m_glfwWindowHandler, keyCallback);

}

Engine::~Engine()
{
    glfwDestroyWindow(m_glfwWindowHandler);
    glfwTerminate();
}

void Engine::run()
{
    std::thread(&Renderer::Renderer::run, m_renderer).detach();
    m_pythonModule->start();
    while (!glfwWindowShouldClose(m_glfwWindowHandler)) // TODO: replace this
    {
        glfwPollEvents();
    }
    m_pythonModule->stop();
}


void Engine::keyCallback(GLFWwindow* glfwWindowHandler, int key, int scancode, int action, int mods)
{
    Engine &engine = *reinterpret_cast<Engine*>(glfwGetWindowUserPointer(glfwWindowHandler));
    if (action != GLFW_PRESS)
    {
        return;
    }
    engine.m_pythonModule->sendKeyInput(key);
}

void Engine::resizeCallback(GLFWwindow *glfwWindowHandler, int, int)
{
    Engine &engine = *reinterpret_cast<Engine*>(glfwGetWindowUserPointer(glfwWindowHandler));
    engine.m_renderer->resize();
}
