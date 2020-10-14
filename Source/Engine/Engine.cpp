/*
*  Engine.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "Engine.hpp"
#include "Log/Log.hpp"

using namespace Kompot;

//Engine::Engine(int argc, char** argv, const std::string& name, const EngineConfig& config)
//    : m_instanceName(name), m_engineSettings(config)
Engine::Engine()
{
    auto &log = Log::getInstance();
    //const int width = static_cast<int>(m_engineSettings.windowWidth);
    //const int height = static_cast<int>(m_engineSettings.windowHeight);

	//glfwSetWindowTitle(m_glfwWindowHandler, m_instanceName.c_str());
    m_clientSubsystem = new ClientSubsystem();
    //m_world.reset(new World());
    //m_pythonModule = new PythonModule(m_world);
    //m_renderer = new Renderer::Renderer(m_glfwWindowHandler, m_instanceName);

    //log << "Renderer successfully initialized." << std::endl;
}

Engine::~Engine()
{
    delete m_clientSubsystem;
}

void Engine::run()
{
    //m_clientSubsystem
    //std::thread(&Renderer::Renderer::run, m_renderer, m_world).detach();
//    std::thread(&ClientSubsystem::run, m_clientSubsystem).detach();
//    while (!m_clientSubsystem->isNeedToExit())
//    {

//    }
    m_clientSubsystem->run();
}
