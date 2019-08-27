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
        ("width",      "Window width",          640_u32t, &engineConfig.windowWidth)
        ("height",     "Window height",         480_u32t, &engineConfig.windowHeight)
        ("maximized",  "Maximize window",       false,    &engineConfig.isMaximized);

    std::ifstream configFile("engine.ini");
    if (configFile.is_open())
    {
        options.loadFromFile(configFile);
    }
    options.loadFromArguments(argc, argv);
    options.notify();

    Engine engine("Test", engineConfig);
    engine.run();

    return 0;
}
