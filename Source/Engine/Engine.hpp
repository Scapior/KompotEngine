/*
 *  Engine.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include "ClientSubsystem/ClientSubsystem.hpp"
#include "EngineConfig.hpp"
#include <EngineTypes.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

namespace Kompot
{
class Engine
{
public:
    Engine();
    ~Engine();

    void run();

private:
    std::string m_instanceName;
    EngineConfig m_engineSettings;

    ClientSubsystem* m_clientSubsystem = nullptr;
};

} // namespace Kompot
