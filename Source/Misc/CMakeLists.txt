cmake_minimum_required(VERSION 3.12)

set(MISC_SOURCES
    Log.cpp
    OptionsParser/OptionsParser.cpp
)

set(MISC_HEADERS
    Log.hpp
    OptionsParser/OptionsParser.hpp
)

add_library(Misc STATIC
            ${MISC_SOURCES}
            ${MISC_HEADERS}
            ${GLOBAL_INCLUDES}
)

target_compile_features(Misc PUBLIC cxx_std_17)
