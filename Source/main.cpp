/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#include "global.hpp"
#include "Engine/Engine.hpp"
#include "Misc/OptionsParser/OptionsParser.hpp"
#include <fstream>

using namespace KompotEngine;

int main(int argc, char** argv)
{
    std::ios::sync_with_stdio(false);
    EngineConfig engineConfig;

    OptionsParser options;
    options.addOptions()
        ("editmode",   "Run engine as editor",  false,    &engineConfig.isEditMode)
        ("fullscreen", "Fullscreen mode",       false,    &engineConfig.isFullscreen)
        ("width",      "Window width",          640_u16t, &engineConfig.windowWidth)
        ("height",     "Window height",         480_u16t, &engineConfig.windowHeight)
        ("maximized",  "Maximize window",       false,    &engineConfig.isMaximized);

    std::ifstream configFile("engine.ini");
    if (configFile.is_open())
    {
        options.loadFromFile(configFile);
    }
    options.loadFromArguments(argc, argv);
    options.notify();

    Engine engine(argc, argv, "Test", engineConfig);
    engine.run();

    return 0;
}
