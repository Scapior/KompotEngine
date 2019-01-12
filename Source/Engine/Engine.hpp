#pragma once
#include "global.hpp"
#include "GAPI_enum.hpp"
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

namespace KompotEngine {

namespace Renderer
{

class IRenderer
{
};

}

struct EngineConfig
{
    bool editMode;
    bool fullscreen;
    uint32_t windowWidth;
    uint32_t windowHeight;
    Renderer::GAPI gapi;
};

class Engine
{
public:
    Engine(const std::string&, const EngineConfig&);
    ~Engine();

    void run();
private:
    std::string  m_instanceName;
    EngineConfig m_engineSettings;

    GLFWwindow* m_glfwWindowHandler;
};

} // KompotEngine namespace
