/*
*  Engine.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#pragma once

#include <EngineTypes.hpp>
#include "EngineConfig.hpp"
#include "ClientSubsystem/ClientSubsystem.hpp"
#include <string>
#include <sstream>
#include <thread>
#include <memory>

namespace Kompot
{

class Engine
{
public:
    Engine();
    ~Engine();

    void run();
private:
    std::string         m_instanceName;
    EngineConfig        m_engineSettings;

    ClientSubsystem* m_clientSubsystem = nullptr;
};

}
