/*
 *  ShaderCompiler.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "ShaderCompiler.hpp"
#include <spirv-tools/libspirv.hpp>
#include <transfo

using namespace Kompot::ClientSubsystem::Renderer;

ShaderCompiler& ShaderCompiler::get()
{
    static ShaderCompiler shaderCompilerSingltone;
    return shaderCompilerSingltone;
}

const std::vector<std::byte> ShaderCompiler::compile(const std::string shaderCode)
{
    static spvtools::SpirvTools spirvTools(SPV_ENV_VULKAN_1_2);
    std::vector<uint32_t> buffer;
    spirvTools.Assemble(shaderCode, &buffer);
    std::vector<std::byte> result;
    for (const auto bufferQuadByte : buffer)
    {
        union ByteConverter
        {
            uint32_t dword;
            std::byte bytes[4];
        };
        ByteConverter conv;
        conv.dword = bufferQuadByte;
        result.emplace_back()
    }
    return result;
}
