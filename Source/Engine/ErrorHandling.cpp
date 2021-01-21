/*
 *  ErrorHandling.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "ErrorHandling.hpp"
#include <Engine/Log/Log.hpp>

#if __GNUC__ > 10
void KompotEngine::ErrorHandling::exit(std::string_view exitMessage, const std::source_location& location, std::string_view stack);
#else
void Kompot::ErrorHandling::exit(std::string_view exitMessage, std::string_view stack)
#endif
{
    Log::getInstance() << '[' << Log::timeNow() << ']' <<
#if __GNUC__ > 10
        location.file_name() << ':' << location.line() << ':' << location.column() << ' ' <<
#endif
        exitMessage << "\nStack:\n"
                       << stack << std::endl;
    std::exit(1);
}
