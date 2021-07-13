/*
 *  ShaderManager.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include <Engine/ClientSubsystem/Renderer/RenderingCommon.hpp>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>

namespace Kompot::Rendering
{
class ShaderManager
{
public:
    static ShaderManager& get();

    const std::vector<uint32_t> load(const std::filesystem::path& path);

private:
    std::unordered_map<std::filesystem::path::string_type, std::vector<uint32_t>> cache;
};

} // namespace Kompot::Rendering
