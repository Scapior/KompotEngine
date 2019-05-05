#pragma once

#include "global.hpp"
#include "../Renderer/Model.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <memory>
#include <array>
#include <limits>

namespace fs = std::filesystem;

namespace KompotEngine {

namespace IO {

class ModelsLoader
{
public:

    static constexpr std::array<uint8_t, 3> KEM_HEADER   = {0x4b,0x45,0x4d};
    static constexpr std::array<uint8_t, 7> KEM_CONSTANT = {0x49,0x4c,0x55,0x18,0x11,0x20,0x15};
    static constexpr uint8_t KEM_VERSION = 1;
    static constexpr uint64_t KEM_RESERVED_BYTES_COUNT = 5;
    static constexpr uint64_t KEM_RESERVED_BLOCKS_BYTES_COUNT = 2;

    enum class KEM_BLOCK_TYPES : uint8_t
    {
        KEM_BLOCK_TYPE_VERTEX  = 0x01,
        KEM_BLOCK_TYPE_NORMALS = 0x02,
        KEM_BLOCK_TYPE_UV      = 0x03,
        KEM_BLOCK_TYPE_FACES   = 0x04

    };

    static constexpr uint32_t KEM_BLOCK_MAX_VEC3_SIZE = std::numeric_limits<uint32_t>::max() - std::numeric_limits<uint32_t>::max() %  (3 * 4);
    static constexpr uint32_t KEM_BLOCK_MAX_VEC2_SIZE = std::numeric_limits<uint32_t>::max() - std::numeric_limits<uint32_t>::max() %  (2 * 4);

    bool loadFile(const fs::path&);
    std::shared_ptr<Renderer::Model> generateModel();
private:
    std::vector<uint8_t> m_lastFileBytes;
    std::string          m_lastFilePath;
};

} // namespace IO

} // namespace KompotEngine

