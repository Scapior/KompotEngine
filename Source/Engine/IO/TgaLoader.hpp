#pragma once

#include "global.hpp"
#include "ResourcesLoader.hpp"
#include "../Renderer/Image.hpp"
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
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

    std::vector<uint8_t> getLastLoadedTextureBytes();
    VkExtent2D getLastLoadedTextureExtent() const;
private:
    VkExtent2D m_lastImageExtent;

};

} // namespace IO

} // namespace KompotEngine

