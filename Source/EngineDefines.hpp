/*
 *  EngineDefines.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include <EngineTypes.hpp>

static const int8_t ENGINE_VESRION_MAJOR = 0;
static const int8_t ENGINE_VESRION_MINOR = 0;
static const int8_t ENGINE_VESRION_PATCH = 1;
static const char* ENGINE_NAME           = "KompotEngine";

/* Build mode */
#ifndef ENGINE_BUILD_SERVER
    #define ENGINE_BUILD_CLIENT
#endif

#if defined(_DEBUG) || !defined(NDEBUF)
    #define ENGINE_DEBUG
#endif

/* Platform and OS macros */

#if defined(ENGINE_OS_WINDOWS)
    #define debugBreak() (__debugbreak())
#endif

#if defined(ENGINE_OS_LINUX)
    #define debugBreak() (__builtin_trap())
#endif

#define breakPoint(expression) debugBreak()
#define check(expression) \
    if (!((expression)))  \
    {                     \
        debugBreak();     \
    }
#define checkVulkanSuccess(expression) \
    if (const auto result = ((expression)); result != vk::Result::eSuccess)  \
    {                     \
        Log::getInstance() << "Expression \""#expression"\" returns " << vk::to_string(result) << std::endl; \
        debugBreak();     \
    }
#define checkNotNull(expression) check(((expression)) != nullptr)
