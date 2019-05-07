#include "Texture.hpp"

using namespace KompotEngine::Renderer;

Texture::Texture(uint32_t width, uint32_t height, const std::vector<uint8_t> &data)
    : m_height(height), m_width(width), m_data(data) {}

uint8_t *Texture::getImageData()
{
    return m_data.data();
}

VkDeviceSize Texture::getImageSize() const
{
    return m_data.size();
}

uint32_t Texture::getWidth() const
{
    return m_width;
}

uint32_t Texture::getHeight() const
{
    return m_height;
}


