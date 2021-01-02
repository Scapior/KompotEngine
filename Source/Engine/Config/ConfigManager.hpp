/*
 *  ConfigManager.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */


#pragma once
#include <string_view>
#include <filesystem>

namespace Kompot
{
class ConfigManager
{
        public:
    static ConfigManager& getInstance()
    {
        static ConfigManager configManagerSingltone;
        return configManagerSingltone;
    }

    void loadCommandLineArguments(int argc, char** argv);
    void loadConfig(const std::string_view& fileName);
    void loadConfig(const std::filesystem::path& filePath);

        private:
    ConfigManager();
};

} // namespace Kompot
