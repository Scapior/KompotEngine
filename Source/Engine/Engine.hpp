#pragma once

#include "global.hpp"
#include "World.hpp"
#include "WindowSystem/Window.hpp"
#include "PythonModule/PythonModule.hpp"
#include "Renderer/Renderer.hpp"
#include <string>
#include <sstream>
#include <thread>
#include <memory>

namespace KompotEngine
{

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
    Engine(const std::string& name, const EngineConfig& config);
    ~Engine();

    void run();
private:
    std::string         m_instanceName;
    EngineConfig        m_engineSettings;
    Renderer::Renderer *m_renderer = nullptr;
    PythonModule       *m_pythonModule = nullptr;


    std::shared_ptr<World> m_world;
};

} // KompotEngine namespace
