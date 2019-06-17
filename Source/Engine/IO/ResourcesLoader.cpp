#include "ResourcesLoader.hpp"

using namespace KompotEngine::IO;

bool ResourcesLoader::loadFile(const fs::path &filePath)
{
    std::ifstream fileStream(filePath, std::ios::binary);
    if (!fileStream.is_open())
    {
        Log::getInstance() << "ModelsLoader::loadFile: couldn't open file '" << filePath << "'." << std::endl;
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

    m_lastFilePath = filePath.string();

    return true;
}
