#pragma once

#include <global.hpp>
#include "EngineConfig.hpp"
#include <string>
#include <sstream>
#include <thread>
#include <memory>

namespace KompotEngine
{

class Engine
{
public:
    Engine(const std::string& name, const EngineConfig& config);
    ~Engine();

    void run();
private:
    std::string         m_instanceName;
    EngineConfig        m_engineSettings;
};

}
