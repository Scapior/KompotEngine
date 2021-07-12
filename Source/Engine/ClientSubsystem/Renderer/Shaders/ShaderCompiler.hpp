/*
 *  ShaderCompiler.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include <string>
#include <vector>

namespace Kompot::ClientSubsystem::Renderer
{
class ShaderCompiler
{
public:
    static ShaderCompiler& get();

    const std::vector<std::byte> compile(const std::string shaderCode);

private:
    ShaderCompiler();
};

} // namespace Kompot::ClientSubsystem::Renderer
