#include "TgaLoader.hpp"

using namespace KompotEngine::IO;
using namespace KompotEngine::Renderer;

std::vector<uint8_t> TgaLoader::getLastLoadedTextureBytes()
{
    static const std::string imageFileIsIncorrectOrNotSupportedString = "TexturesLoader::loadFile: image file is incorrect or not supported.";

    if (m_lastFileBytes.size() < 18)
    {
        Log::getInstance() << "TexturesLoader::loadFile: Trying to load image before loading data from file or file is incorrect." << std::endl;
        return {};
    }

    const uint8_t colorMapType = m_lastFileBytes[1];
    const uint8_t imageType = m_lastFileBytes[2];
    const uint32_t width = static_cast<uint32_t>(m_lastFileBytes[12]) | (static_cast<uint32_t>(m_lastFileBytes[13]) << 8);
    const uint32_t height = static_cast<uint32_t>(m_lastFileBytes[14]) | (static_cast<uint32_t>(m_lastFileBytes[15]) << 8);
    const uint8_t imageBitsPerPixel = m_lastFileBytes[16];

    const auto pixelsCount = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
    const uint64_t offset = 18 + m_lastFileBytes[0]; //first byte is id field size
    const uint64_t offsetToImageEnd = pixelsCount * static_cast<uint64_t>(imageBitsPerPixel / 8) + offset;

    if (colorMapType != NO_COLOR_MAP_TYPE || imageType != UNCOMPRESSED_TRUE_COLOR ||
       (imageBitsPerPixel != 32 && imageBitsPerPixel != 24)|| // only uncompressed true-color 24/32 bits image
        offsetToImageEnd > m_lastFileBytes.size())
    {
        Log::getInstance() << imageFileIsIncorrectOrNotSupportedString << std::endl;
        return {};
    }

    std::vector<uint8_t> pixelsBytes(pixelsCount * 4_u64t);
    for (auto i = 0_u64t, j = offset; j < offsetToImageEnd;)
    {
        uint8_t bgr[3];
        for (auto &byte : bgr)
        {
            byte = m_lastFileBytes[j++];
        }
        pixelsBytes[i++] = bgr[2]; // swap R
        pixelsBytes[i++] = bgr[1]; // and
        pixelsBytes[i++] = bgr[0]; // B
        if (imageBitsPerPixel == 24) // alpha
        {
            pixelsBytes[i++] = 0xff;
        }
        else
        {
            pixelsBytes[i++] = m_lastFileBytes[j++];;
        }
    }
    m_lastImageExtent.width = width;
    m_lastImageExtent.height = height;
    return pixelsBytes;
}

VkExtent2D TgaLoader::getLastLoadedTextureExtent() const
{
    return m_lastImageExtent;
}

