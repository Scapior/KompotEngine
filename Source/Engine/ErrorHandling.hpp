/*
 *  ErrorHandling.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include <string_view>
#if __GNUC__ > 10
#include <source_location>
#endif
#include "DebugUtils/DebugUtils.hpp"


namespace Kompot::ErrorHandling
{

#if __GNUC__ > 10
void exit(std::string_view exitMessage, const std::source_location& location = std::source_location::current(), std::string_view stack = DebugUtils::getCallstack());
#else
void exit(std::string_view exitMessage,  std::string_view stack = DebugUtils::getCallstack());
#endif

} // namespace Kompot
