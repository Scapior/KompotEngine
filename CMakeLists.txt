cmake_minimum_required(VERSION 3.12)

project(KompotEngine VERSION 0.0.0.1 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
if (WIN32)
    list(APPEND CMAKE_CXX_FLAGS "/EHsc /std:c++17")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${PROJECT_SOURCE_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
else()
    list(APPEND CMAKE_CXX_FLAGS "-pthread -fno-exceptions -std=c++17 ${CMAKE_CXX_FLAGS}")
endif()

#add_subdirectory(Shaders)
#add_dependencies(KompotEngine Shaders)

add_subdirectory(Source)
