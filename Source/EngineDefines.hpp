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
static const char * ENGINE_NAME = "KompotEngine";

/* Build mode */
#ifndef ENGINE_BUILD_SERVER
    #define ENGINE_BUILD_CLIENT
#endif

#if defined(_DEBUG) || !defined(NDEBUF)
    #define ENGINE_DEBUG
#endif

/* Platform and OS macros */

#if defined(_WIN32) || defined(__WIN32__)
    #define ENGINE_OS_WINDOWS
    #if defined(_WIN64) || defined(WIN64)
        #define ENGINE_PLATFORM_x64
        #define ENGINE_OS_WINDOWS_x64
    #else
        #define ENGINE_PLATFORM_x86
        #define ENGINE_OS_WINDOWS_x32
    #endif
#endif

#if defined(__linux__)
    #define ENGINE_OS_LINUX
    #if defined(__x86_64__)
        #define ENGINE_PLATFORM_x64
        #define ENGINE_OS_LINUX_x64
    #elif defined(__i386__)
        #define ENGINE_PLATFORM_x86
        #define ENGINE_OS_LINUX_x32
    #endif
#endif

#if defined(ENGINE_OS_WINDOWS)
    #define debugBreak() (__debugbreak())
#endif

#if defined(ENGINE_OS_LINUX)
    #define debugBreak() (__builtin_trap())
#endif

#define breakPoint(expression) debugBreak()
#define check(expression) if(!((expression))) { debugBreak(); }
#define checkNotNull(expression) check(((expression)) != nullptr)
