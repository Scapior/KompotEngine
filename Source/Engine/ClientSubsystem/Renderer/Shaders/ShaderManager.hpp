/*
 *  ShaderManager.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include <Engine/ClientSubsystem/Renderer/IRenderer.hpp>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>

namespace Kompot::ClientSubsystem::Renderer
{

class ShaderManager
{
public:
    static ShaderManager& get();

    const std::vector<std::byte> load(const std::filesystem::path& path);

private:
    ShaderManager();

    std::unordered_map<std::wstring, std::vector<std::byte>> cache;
};

} // namespace Kompot::ClientSubsystem::Renderer
