cmake_minimum_required(VERSION 3.12)

set(MISC_SOURCES
    Log.cpp
    Guid.cpp
    OptionsParser/OptionsParser.cpp
    DebugUtils/DebugUtils.cpp
    StringUtils/StringUtils.cpp
)

set(MISC_HEADERS
    Log.hpp
    Guid.hpp
    OptionsParser/OptionsParser.hpp
    DebugUtils/DebugUtils.hpp
    Templates/Functions.hpp
    DateTimeFormatter.hpp
    StringUtils/StringUtils.hpp
)

add_library(Misc STATIC
            ${MISC_SOURCES}
            ${MISC_HEADERS}
            ${GLOBAL_INCLUDES}
)

target_compile_features(Misc PUBLIC cxx_std_17)
