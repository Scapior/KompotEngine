#pragma once

#include "global.hpp"
#include "ResourcesLoader.hpp"
#include "../Renderer/Texture.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <memory>
#include <array>
#include <limits>

namespace fs = std::filesystem;

namespace KompotEngine {

namespace IO {

class TgaLoader : public ResourcesLoader
{
public:
    static const uint8_t NO_COLOR_MAP_TYPE = 0;
    static const uint8_t UNCOMPRESSED_TRUE_COLOR = 2;

    std::shared_ptr<Renderer::Texture> generateTexture();
};

} // namespace IO

} // namespace KompotEngine

