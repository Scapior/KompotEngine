/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#pragma once

#include <global.hpp>
#include "EngineConfig.hpp"
#include "ClientSubsystem/ClientSubsystem.hpp"
#include <string>
#include <sstream>
#include <thread>
#include <memory>

namespace KompotEngine
{

class Engine
{
public:
    Engine(int argc, char** argv, const std::string& name, const EngineConfig& config);
    ~Engine();

    void run();
private:
    std::string         m_instanceName;
    EngineConfig        m_engineSettings;

    ClientSubsystem* m_clientSubsystem = nullptr;
};

}
