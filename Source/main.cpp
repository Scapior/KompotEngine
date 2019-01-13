#include "global.hpp"
#include "Engine/Engine.hpp"
#include "Misc/ProgramOptions/ProgramOptions.hpp"
#include <fstream>

using namespace KompotEngine;

int main(int argc, char **argv)
{
    EngineConfig engineConfig;

    ProgramOptions options;
    options.addOptions()
        ("editmode",   "Run engine as editor", false,               &engineConfig.editMode)
        ("fullscreen", "Fullscreen mode",      false,               &engineConfig.fullscreen)
        ("width",      "Window width",         640_u32t,            &engineConfig.windowWidth)
        ("height",     "Window height",        480_u32t,            &engineConfig.windowHeight);

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
