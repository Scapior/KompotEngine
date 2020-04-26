cmake_minimum_required(VERSION 3.12)

set(MATH_SOURCES
    Vector.cpp
)

set(MATH_HEADERS
    Vector.hpp
)

add_library(Math STATIC
            ${MATH_SOURCES}
            ${MATH_HEADERS}
)

target_compile_features(Math PUBLIC cxx_std_17)
