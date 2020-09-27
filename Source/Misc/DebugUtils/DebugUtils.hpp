/*
*  DebugMacros.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include <string>
#include <EngineDefines.hpp>

#if defined(ENGINE_OS_WINDOWS)
    #define debugBreak() (__debugbreak())
#endif

#if defined(ENGINE_OS_LINUX)
    #define debugBreak() (__builtin_trap())
#endif

#define breakPoint(expression) debugBreak()
#define check(expression) if(!((expression))) { debugBreak(); }
#define checkNotNull(expression) check(((expression)) != 0)

namespace DebugUtils
{

    std::string getLastPlatformError();

    std::string getCallstack();

    template<typename T>
    void PrintCallstack(T& outputStream)
    {
        outputStream << getCallstack();
    }

}

#pragma once
