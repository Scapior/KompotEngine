#pragma once

#include "global.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <memory>
#include <array>
#include <limits>

namespace fs = std::filesystem;

namespace KompotEngine
{

namespace IO
{

class ResourcesLoader
{
public:
    bool loadFile(const fs::path&);
protected:
    ResourcesLoader() = default;
    std::vector<uint8_t> m_lastFileBytes;
    std::string          m_lastFilePath;
};

} // namespace IO

} // namespace KompotEngine

