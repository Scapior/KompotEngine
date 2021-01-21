/*
 *  DebugMacros.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include <EngineDefines.hpp>
#include <string>

namespace DebugUtils
{
std::string getLastPlatformError();

std::string getCallstack();

template<typename T>
void PrintCallstack(T& outputStream)
{
    outputStream << getCallstack();
}

} // namespace DebugUtils

#pragma once
