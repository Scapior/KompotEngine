/*
 *  ShaderCompiler.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "ShaderCompiler.hpp"
#include <glslang/SPIRV/GlslangToSpv.h>
#include <filesystem>
#include <fstream>
#include "Engine/Log/Log.hpp"

using namespace Kompot::Rendering;
namespace fs = std::filesystem;

ShaderCompiler::ShaderCompiler()
{
    glslang::InitializeProcess();
}

ShaderCompiler::~ShaderCompiler()
{
    glslang::FinalizeProcess();
}

ShaderCompiler& ShaderCompiler::get()
{
    static ShaderCompiler shaderCompilerSingltone;
    return shaderCompilerSingltone;
}

EShLanguage detectShaderType(const fs::path& path)
{
    if (path.extension() == ".vert")
    {
        return EShLangVertex;
    }
    if (path.extension() == ".frag")
    {
        return EShLangFragment;
    }
    if (path.extension() == ".tesc")
    {
        return EShLangTessControl;
    }
    if (path.extension() == ".tese")
    {
        return EShLangTessEvaluation;
    }
    if (path.extension() == ".geom")
    {
        return EShLangGeometry;
    }

    return EShLangCount;
}

inline void initResource(TBuiltInResource& resource)
{
    resource.maxLights                                 = 32;
    resource.maxClipPlanes                             = 6;
    resource.maxTextureUnits                           = 32;
    resource.maxTextureCoords                          = 32;
    resource.maxVertexAttribs                          = 64;
    resource.maxVertexUniformComponents                = 4096;
    resource.maxVaryingFloats                          = 64;
    resource.maxVertexTextureImageUnits                = 32;
    resource.maxCombinedTextureImageUnits              = 80;
    resource.maxTextureImageUnits                      = 32;
    resource.maxFragmentUniformComponents              = 4096;
    resource.maxDrawBuffers                            = 32;
    resource.maxVertexUniformVectors                   = 128;
    resource.maxVaryingVectors                         = 8;
    resource.maxFragmentUniformVectors                 = 16;
    resource.maxVertexOutputVectors                    = 16;
    resource.maxFragmentInputVectors                   = 15;
    resource.minProgramTexelOffset                     = -8;
    resource.maxProgramTexelOffset                     = 7;
    resource.maxClipDistances                          = 8;
    resource.maxComputeWorkGroupCountX                 = 65535;
    resource.maxComputeWorkGroupCountY                 = 65535;
    resource.maxComputeWorkGroupCountZ                 = 65535;
    resource.maxComputeWorkGroupSizeX                  = 1024;
    resource.maxComputeWorkGroupSizeY                  = 1024;
    resource.maxComputeWorkGroupSizeZ                  = 64;
    resource.maxComputeUniformComponents               = 1024;
    resource.maxComputeTextureImageUnits               = 16;
    resource.maxComputeImageUniforms                   = 8;
    resource.maxComputeAtomicCounters                  = 8;
    resource.maxComputeAtomicCounterBuffers            = 1;
    resource.maxVaryingComponents                      = 60;
    resource.maxVertexOutputComponents                 = 64;
    resource.maxGeometryInputComponents                = 64;
    resource.maxGeometryOutputComponents               = 128;
    resource.maxFragmentInputComponents                = 128;
    resource.maxImageUnits                             = 8;
    resource.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    resource.maxCombinedShaderOutputResources          = 8;
    resource.maxImageSamples                           = 0;
    resource.maxVertexImageUniforms                    = 0;
    resource.maxTessControlImageUniforms               = 0;
    resource.maxTessEvaluationImageUniforms            = 0;
    resource.maxGeometryImageUniforms                  = 0;
    resource.maxFragmentImageUniforms                  = 8;
    resource.maxCombinedImageUniforms                  = 8;
    resource.maxGeometryTextureImageUnits              = 16;
    resource.maxGeometryOutputVertices                 = 256;
    resource.maxGeometryTotalOutputComponents          = 1024;
    resource.maxGeometryUniformComponents              = 1024;
    resource.maxGeometryVaryingComponents              = 64;
    resource.maxTessControlInputComponents             = 128;
    resource.maxTessControlOutputComponents            = 128;
    resource.maxTessControlTextureImageUnits           = 16;
    resource.maxTessControlUniformComponents           = 1024;
    resource.maxTessControlTotalOutputComponents       = 4096;
    resource.maxTessEvaluationInputComponents          = 128;
    resource.maxTessEvaluationOutputComponents         = 128;
    resource.maxTessEvaluationTextureImageUnits        = 16;
    resource.maxTessEvaluationUniformComponents        = 1024;
    resource.maxTessPatchComponents                    = 120;
    resource.maxPatchVertices                          = 32;
    resource.maxTessGenLevel                           = 64;
    resource.maxViewports                              = 16;
    resource.maxVertexAtomicCounters                   = 0;
    resource.maxTessControlAtomicCounters              = 0;
    resource.maxTessEvaluationAtomicCounters           = 0;
    resource.maxGeometryAtomicCounters                 = 0;
    resource.maxFragmentAtomicCounters                 = 8;
    resource.maxCombinedAtomicCounters                 = 8;
    resource.maxAtomicCounterBindings                  = 1;
    resource.maxVertexAtomicCounterBuffers             = 0;
    resource.maxTessControlAtomicCounterBuffers        = 0;
    resource.maxTessEvaluationAtomicCounterBuffers     = 0;
    resource.maxGeometryAtomicCounterBuffers           = 0;
    resource.maxFragmentAtomicCounterBuffers           = 1;
    resource.maxCombinedAtomicCounterBuffers           = 1;
    resource.maxAtomicCounterBufferSize                = 16384;
    resource.maxTransformFeedbackBuffers               = 4;
    resource.maxTransformFeedbackInterleavedComponents = 64;
    resource.maxCullDistances                          = 8;
    resource.maxCombinedClipAndCullDistances           = 8;
    resource.maxSamples                                = 4;
    resource.maxMeshOutputVerticesNV                   = 256;
    resource.maxMeshOutputPrimitivesNV                 = 512;
    resource.maxMeshWorkGroupSizeX_NV                  = 32;
    resource.maxMeshWorkGroupSizeY_NV                  = 1;
    resource.maxMeshWorkGroupSizeZ_NV                  = 1;
    resource.maxTaskWorkGroupSizeX_NV                  = 32;
    resource.maxTaskWorkGroupSizeY_NV                  = 1;
    resource.maxTaskWorkGroupSizeZ_NV                  = 1;
    resource.maxMeshViewCountNV                        = 4;

    resource.limits.nonInductiveForLoops                 = 1;
    resource.limits.whileLoops                           = 1;
    resource.limits.doWhileLoops                         = 1;
    resource.limits.generalUniformIndexing               = 1;
    resource.limits.generalAttributeMatrixVectorIndexing = 1;
    resource.limits.generalVaryingIndexing               = 1;
    resource.limits.generalSamplerIndexing               = 1;
    resource.limits.generalVariableIndexing              = 1;
    resource.limits.generalConstantMatrixVectorIndexing  = 1;
}

std::vector<uint32_t> ShaderCompiler::compile(const fs::path shaderCodePath, const std::filesystem::path cachePath)
{
    std::ifstream shaderFile(shaderCodePath);
    std::string shaderText((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());

    const EShLanguage shaderStage = detectShaderType(shaderCodePath);
    glslang::TShader shader(shaderStage);
    shader.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan, 460);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);

    glslang::TProgram program;
    TBuiltInResource resource{};
    initResource(resource);

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

    const char* shaderStrings[1];
    shaderStrings[0] = shaderText.c_str();
    shader.setStrings(shaderStrings, 1);

    if (!shader.parse(&resource, 100, false, messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return {};
    }

    program.addShader(&shader);

    if (!program.link(messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        fflush(stdout);
        return {};
    }

    std::vector<uint32_t> spirvBytecode;
    glslang::GlslangToSpv(*program.getIntermediate(shaderStage), spirvBytecode);

    if (std::error_code error; !fs::create_directories("Cache/Shaders", error) && error)
    {
        Log::getInstance() << "Failed to create shaders cache directory: " << error.message()  << std::endl;
    }

    const std::string cachePathString(cachePath.native().begin(), cachePath.native().end());
    glslang::OutputSpvBin(spirvBytecode, cachePathString.c_str());

    return spirvBytecode;
}

