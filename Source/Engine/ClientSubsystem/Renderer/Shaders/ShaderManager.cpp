/*
 *  ShaderManager.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "ShaderManager.hpp"
#include "ShaderCompiler.hpp"
#include <Engine/Log/Log.hpp>
#include <Engine/ErrorHandling.hpp>
#include <fstream>

using namespace Kompot;
using namespace Kompot::ClientSubsystem::Renderer;

using ibytestream = std::basic_ifstream<std::byte>;
using obytestream = std::basic_ofstream<std::byte>;

const inline std::filesystem::path toCachePath(const std::filesystem::path& path)
{
    return "Cache" / path / ".spv";
}

ShaderManager &ShaderManager::get()
{
    static ShaderManager shaderManagerSingnltone;
    return shaderManagerSingnltone;
}

const std::vector<std::byte> loadCacheFile(const std::filesystem::path& path)
{
    ibytestream file{path, std::ios::ate | std::ios::binary};
    if (!file.is_open())
    {
        return {};
    }
    const auto fileSize = file.tellg();
    std::vector<std::byte> buffer(static_cast<unsigned long>(fileSize));
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

void saveCache(const std::filesystem::path& path, const std::vector<std::byte> bytecode)
{
    obytestream file{path, std::ios::binary};
    if (!file.is_open())
    {
        Log::getInstance() << "Failed to save a shader bytecode to cache file \"" << path << "\"";
        return;
    }
    file.write(bytecode.data(), bytecode.size());
}

const std::vector<char> loadFile(const std::filesystem::path &path)
{
    std::ifstream file{path, std::ios::ate | std::ios::binary};
    if (!file.is_open())
    {
        Log::getInstance() << "File \"" << path << "\" not found!";
        return {};
    }
    const auto fileSize = file.tellg();
    std::vector<char> buffer(static_cast<unsigned long>(fileSize));
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}


const std::vector<std::byte> ShaderManager::load(const std::filesystem::path &path)
{
    if (path.is_absolute())
    {
        Log::getInstance() << path << " - path must be relative!" << std::endl;
        return {};
    }

    const auto cachePath = toCachePath(path);

    if (const auto cacheValue = cache.find(cachePath); cacheValue != cache.end())
    {
        return cacheValue->second;
    }

    if (const auto shaderBinary = loadCacheFile(path); !shaderBinary.empty())
    {
        cache.emplace(cachePath, shaderBinary);
        return shaderBinary;
    }

    const auto shaderText = loadFile(path);
    if (shaderText.empty())
    {
        Kompot::ErrorHandling::exit("Failed to load a shader file.");
    }
    const auto shaderBinary = ShaderCompiler::get().compile(shaderText);
    cache.emplace(cachePath, shaderBinary);
    saveCache(cachePath, shaderBinary);

    return shaderBinary;
}
