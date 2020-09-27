cmake_minimum_required(VERSION 3.12)

set(MISC_SOURCES
    Guid.cpp
    OptionsParser/OptionsParser.cpp
    StringUtils/StringUtils.cpp
)

set(MISC_HEADERS
    Guid.hpp
    OptionsParser/OptionsParser.hpp
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
