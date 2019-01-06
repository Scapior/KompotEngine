#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

namespace boost_options = boost::program_options;

struct EngineConfig
{
    bool editMode;
    bool fullscreen;
    uint32_t windowWidth;
    uint32_t windowHeight;
};

int main(int argc, char **argv)
{
    EngineConfig engineConfig;

    boost_options::options_description description;
    boost_options::variables_map optionsMap;

    description.add_options()
            ("help",        "Show this help screen")
            ("editmode",    boost_options::bool_switch(&engineConfig.editMode)->default_value(false),           "Run engine as editor")
            ("fullscreen",  boost_options::bool_switch(&engineConfig.fullscreen)->default_value(false),         "Fullscreen mode")
            ("width",       boost_options::value<uint32_t>(&engineConfig.windowWidth)->default_value(640u),     "Window width")
            ("height",      boost_options::value<uint32_t>(&engineConfig.windowHeight)->default_value(480u),    "Window height");


    std::ifstream configFile("engine.ini");
    if (configFile.is_open())
    {
        boost_options::store(boost_options::parse_config_file(configFile, description), optionsMap);
        boost_options::notify(optionsMap);
    }

    auto optionsParser = boost_options::command_line_parser(argc, argv).options(description);
    const auto parsedOptions = optionsParser.allow_unregistered().run();
    boost_options::store(parsedOptions, optionsMap);
    boost_options::notify(optionsMap);

    if (optionsMap.count("help"))
    {
        std::cout << description << std::endl;
        return 0;
    }

    std::cout << "Width: " << engineConfig.windowWidth << std::endl;
    std::cout << "Height: " << engineConfig.windowHeight << std::endl;

    // create engine and set config here

    return 0;
}
