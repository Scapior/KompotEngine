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
using namespace Kompot::Rendering;

namespace fs = std::filesystem;

const inline fs::path toCachePath(const fs::path& path)
{
    return "Cache/" + path.string() + ".spv";
}

ShaderManager& ShaderManager::get()
{
    static ShaderManager shaderManagerSingnltone;
    return shaderManagerSingnltone;
}

std::vector<uint32_t> loadCacheFile(const fs::path& path)
{
#if defined(__GNUC__)
    std::ifstream file{path, std::ios::binary | std::ios::ate};
    if (!file.is_open())
    {
        return {};
    }
    const auto size = file.tellg();
    std::vector<uint32_t> buffer(size / 4);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
#else
    std::basic_ifstream<uint32_t> file{path, std::ios::ate | std::ios::binary};
    if (!file.is_open())
    {
        return {};
    }
    std::vector<uint32_t> buffer{std::istreambuf_iterator<uint32_t>(file), std::istreambuf_iterator<uint32_t>()};
#endif
    return buffer;
}

const std::vector<uint32_t> ShaderManager::load(const std::filesystem::path& path)
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

    if (const auto shaderBinary = loadCacheFile(cachePath); !shaderBinary.empty())
    {
        cache.emplace(cachePath, shaderBinary);
        return shaderBinary;
    }

    const auto shaderBinary = ShaderCompiler::get().compile(path, cachePath);
    cache.emplace(cachePath, shaderBinary);

    return shaderBinary;
}
