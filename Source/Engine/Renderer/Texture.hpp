#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>

namespace KompotEngine
{

namespace Renderer
{

class Texture
{
public:
    Texture(uint32_t, uint32_t, const std::vector<uint8_t>&);
    uint8_t *getImageData();
    VkDeviceSize getImageSize() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
private:
    std::vector<uint8_t> m_data;
    uint32_t m_height;
    uint32_t m_width;
};

} //namespace Renderer

} // namespace KompotEngine
