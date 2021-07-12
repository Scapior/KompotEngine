/*
 *  ShaderCompiler.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include <Engine/ClientSubsystem/Renderer/IRenderer.hpp>

namespace Kompot::ClientSubsystem::Renderer
{
class ShaderCompiler
{
public:
    void Compile(IShader* Shader);

private:
    ShaderCompiler();
};

} // namespace Kompot::ClientSubsystem::Renderer
