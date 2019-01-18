#pragma once

#include "global.hpp"
#include "Renderer/Renderer.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <thread>

namespace KompotEngine {

struct EngineConfig
{
    bool isEditMode;
    bool isFullscreen;
    uint32_t windowWidth;
    uint32_t windowHeight;
    bool isMaximized;
};

class Engine
{
public:
    Engine(const std::string&, const EngineConfig&);
    ~Engine();

    void run();
private:
    std::string         m_instanceName;
    EngineConfig        m_engineSettings;
    Renderer::Renderer *m_renderer = nullptr;

    GLFWwindow* m_glfwWindowHandler;

    // GLFW callbacks

    void keyCallback(GLFWwindow*, int, int, int, int);
    void characterCallback(GLFWwindow*, unsigned int, int);
};

} // KompotEngine namespace
