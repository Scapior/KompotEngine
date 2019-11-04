/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#include "Engine.hpp"

/*!
    \class Engine
    \brief The Engine class is a root class that contains all subsystems of the engine.


    Engine copy paste: defines a cache that stores objects of type T
    associated with keys of type Key. For example, here's the
    definition of a cache that stores objects of type Employee
    associated with an integer key:

    \snippet code/doc_src_qcache.cpp 0

    Here's how to insert an object in the cache:

    \snippet code/doc_src_qcache.cpp 1

    ... detailed description ommitted

    \sa Engine
*/

using namespace KompotEngine;


/*!
  \fn Engine::Engine(const std::string& name, const EngineConfig& config)

  Removes \a n characters from the string, starting at the given \a
  position index, and returns a reference to the string.

  If the specified \a position index is within the string, but \a
  position + \a n is beyond the end of the string, the string is
  truncated at the specified \a position.

  \snippet Engine/Engine.cpp 41

  \sa Engine()
*/
Engine::Engine(int argc, char** argv, const std::string& name, const EngineConfig& config)
    : m_instanceName(name), m_engineSettings(config)
{
    auto &log = Log::getInstance();
    //const int width = static_cast<int>(m_engineSettings.windowWidth);
    //const int height = static_cast<int>(m_engineSettings.windowHeight);

	//glfwSetWindowTitle(m_glfwWindowHandler, m_instanceName.c_str());
    m_clientSubsystem = new ClientSubsystem(argc, argv, config);
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
    std::thread(&ClientSubsystem::run, m_clientSubsystem).detach();
}
