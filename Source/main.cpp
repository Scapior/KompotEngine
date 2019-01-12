#include "global.hpp"
#include "Engine/Engine.hpp"
#include "Misc/ProgramOptions/ProgramOptions.hpp"
#include <iostream>
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
        ("height",     "Window height",        480_u32t,            &engineConfig.windowHeight)
        ("gapi",       "Graphic API",          Renderer::GAPI::OGL, &engineConfig.gapi);

    std::ifstream configFile("engine.ini");
    if (configFile.is_open())
    {
        options.loadFromFile(configFile);
    }
    options.loadFromArguments(argc, argv);
    options.notify();

    //Engine engine("Test", engineConfig);
    //engine.run();

    std::cout << " Edit Mode:" << engineConfig.editMode
              << "\nFullscreen:" << engineConfig.fullscreen
              << "\n     Width:" << engineConfig.windowWidth
              << "\n    Height:" << engineConfig.windowHeight
              << "\n      GAPI:" << engineConfig.gapi << std::endl;

    return 0;
}
