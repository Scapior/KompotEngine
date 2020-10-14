/*
*  ConfigManager.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/


#include "ConfigManager.hpp"

using namespace Kompot;

void ConfigManager::loadCommandLineArguments(int argc, char** argv)
{
    //EngineConfig engineConfig;
//    OptionsParser options;
//    options.addOptions()
//        ("editmode",   "Run engine as editor",  false,    &engineConfig.isEditMode)
//        ("fullscreen", "Fullscreen mode",       false,    &engineConfig.isFullscreen)
//        ("width",      "Window width",          640_u16t, &engineConfig.windowWidth)
//        ("height",     "Window height",         480_u16t, &engineConfig.windowHeight)
//        ("maximized",  "Maximize window",       false,    &engineConfig.isMaximized);

//    std::ifstream configFile("engine.ini");
//    if (configFile.is_open())
//    {
//        options.loadFromFile(configFile);
//    }
//    options.loadFromArguments(argc, argv);
    //    options.notify();
}

void ConfigManager::loadConfig(const std::string_view& fileName)
{

}

void ConfigManager::loadConfig(const std::filesystem::path& filePath)
{

}

ConfigManager::ConfigManager()
{

}
