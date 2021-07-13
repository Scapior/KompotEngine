/*
 *  ShaderCompiler.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include <Engine/ClientSubsystem/Renderer/RenderingCommon.hpp>
#include <string>
#include <vector>
#include <filesystem>

namespace Kompot::Rendering
{
class ShaderCompiler
{
public:
    static ShaderCompiler& get();

    std::vector<uint32_t> compile(const std::filesystem::path shaderCodePath, const std::filesystem::path cachePath);

private:
    ShaderCompiler();
    ~ShaderCompiler();
};

} // namespace Kompot::ClientSubsystem::Renderer
