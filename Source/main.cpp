/*
*  main.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include <Engine/Engine.hpp>
#include <Engine/Config/ConfigManager.hpp>

int main(int argc, char** argv)
{
    std::ios::sync_with_stdio(false);

    using Kompot::ConfigManager;
    ConfigManager& configManager = ConfigManager::getInstance();
    configManager.loadCommandLineArguments(argc, argv);

    Kompot::Engine engine;
    engine.run();

    return 0;
}
