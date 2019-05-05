#include "ModelsLoader.hpp"

using namespace KompotEngine::IO;
using namespace KompotEngine::Renderer;

bool ModelsLoader::loadFile(const fs::path &filePath)
{
    std::ifstream fileStream(filePath, std::ios::binary);
    if (!fileStream.is_open())
    {
        Log::getInstance() << " ModelsLoader::loadFile: couldn't open file '" << filePath << "'." << std::endl;
        return false;
    }

    fileStream.unsetf(std::ios_base::skipws);
    m_lastFileBytes.clear();

    while (true)
    {
        uint8_t byte;
        fileStream >> byte;
        if (fileStream.eof())
        {
            break;
        }
        m_lastFileBytes.push_back(byte);
    }

    if (m_lastFileBytes.size() < 16)
    {
        Log::getInstance() << " ModelsLoader::loadFile: file '" << filePath << "' too short for specifid format." << std::endl;
        m_lastFileBytes.clear();
        return false;
    }

    m_lastFilePath = filePath;

    return true;
}


std::shared_ptr<Model> ModelsLoader::generateModel()
{
    if (m_lastFileBytes.size() == 0)
    {
        Log::getInstance() << " ModelsLoader::loadFile: Trying to generate model before loading data from file." << std::endl;
        m_lastFileBytes.clear();
        return {};
    }
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t>  verticesIndices;
    std::vector<glm::vec3> verticesNormals;
    std::vector<glm::vec2> verticesUv;

    auto i = 0_u64t;

    // check header, contant and format version

    for (const auto &headerByte : KEM_HEADER)
    {
        if (m_lastFileBytes[i++] != headerByte)
        {
            Log::getInstance() << " ModelsLoader::loadFile: file " << m_lastFilePath << " - wrong header." << std::endl;
            m_lastFileBytes.clear();
            return {};
        }
    }

    for (const auto &constantByte : KEM_CONSTANT)
    {
        if (m_lastFileBytes[i++] != constantByte)
        {
            Log::getInstance() << " ModelsLoader::loadFile: file '" << m_lastFilePath << "' - wrong constant." << std::endl;
            m_lastFileBytes.clear();
            return {};
        }
    }

    if (m_lastFileBytes[i++] > KEM_VERSION)
    {
        Log::getInstance() << " ModelsLoader::loadFile: file '" << m_lastFilePath << "' - unsupported version of format." << std::endl;
        m_lastFileBytes.clear();
        return {};
    }
    i += KEM_RESERVED_BYTES_COUNT;

    while ( i < m_lastFileBytes.size())
    {
        if (i + 8 >= m_lastFileBytes.size())
        {
            Log::getInstance() << " ModelsLoader::loadFile: file '" << m_lastFilePath << "' not correct." << std::endl;
            m_lastFileBytes.clear();
            return {};
        }
        KEM_BLOCK_TYPES blockType = static_cast<KEM_BLOCK_TYPES>(m_lastFileBytes[i++]);
        uint8_t flags = m_lastFileBytes[i++];
        i += KEM_RESERVED_BLOCKS_BYTES_COUNT;
        uint32_t blockDataSize = (static_cast<uint32_t>(m_lastFileBytes[i++]) << 24) |
                                 (static_cast<uint32_t>(m_lastFileBytes[i++]) << 16) |
                                 (static_cast<uint32_t>(m_lastFileBytes[i++]) << 8) |
                                  static_cast<uint32_t>(m_lastFileBytes[i++]);
        if (i + blockDataSize  > m_lastFileBytes.size())
        {
            Log::getInstance() << "ModelsLoader::loadFile: file '" << m_lastFilePath << "' not correct." << std::endl;
            m_lastFileBytes.clear();
            return {};
        }
        const auto lastDataIndex = i + blockDataSize;
        if (blockType == KEM_BLOCK_TYPES::KEM_BLOCK_TYPE_VERTEX || blockType == KEM_BLOCK_TYPES::KEM_BLOCK_TYPE_NORMALS)
        {
            while (i < lastDataIndex)
            {
                glm::vec3 dataVector;
                void *bytesData = &m_lastFileBytes[i];
                memcpy(&dataVector.data, bytesData, sizeof(dataVector));
                i += sizeof(dataVector);
                if (blockType == KEM_BLOCK_TYPES::KEM_BLOCK_TYPE_VERTEX)
                {
                    vertices.push_back(dataVector);
                }
                else
                {
                    verticesNormals.push_back(dataVector);
                }
            }
        }
        else if (blockType == KEM_BLOCK_TYPES::KEM_BLOCK_TYPE_UV)
        {
            while (i < lastDataIndex)
            {
                glm::vec2 dataVector;
                void *bytesData = &m_lastFileBytes[i];
                memcpy(&dataVector.data, bytesData, sizeof(dataVector));
                i += sizeof(dataVector);
                verticesUv.push_back(dataVector);
            }
        }
        else if (blockType == KEM_BLOCK_TYPES::KEM_BLOCK_TYPE_FACES)
        {
            while (i < lastDataIndex)
            {
                uint32_t index = (static_cast<uint32_t>(m_lastFileBytes[i++]) << 24) |
                                         (static_cast<uint32_t>(m_lastFileBytes[i++]) << 16) |
                                         (static_cast<uint32_t>(m_lastFileBytes[i++]) << 8) |
                                          static_cast<uint32_t>(m_lastFileBytes[i++]);
                verticesIndices.push_back(index);
            }
        }
    }

    return std::make_shared<Model>(Model(vertices, verticesIndices, verticesNormals, verticesUv));
}
