cmake_minimum_required(VERSION 3.12)
list(APPEND CMAKE_CXX_FLAGS "-pthread -std=c++17 ${CMAKE_CXX_FLAGS} -fno-exceptions")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../ )
project(KompotEngine)

set(SOURCES
    Source/main.cpp
    Source/Engine/Engine.cpp
    Source/Engine/Renderer/Renderer.cpp
    Source/Misc/TickingObject.cpp
    Source/Misc/ProgramOptions/ProgramOptions.cpp)

set(HEADERS
    Source/global.hpp
    Source/Engine/Engine.hpp
    Source/Engine/Renderer/Renderer.hpp
    Source/Misc/Log.hpp
    Source/Misc/TickingObject.hpp
    Source/Misc/ProgramOptions/ProgramOptions.hpp)

find_package(PythonLibs REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
include_directories(
        ${GLFW_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIRS}
)
link_directories(
    ${PYTHON_LIBRARIES}
)

add_executable(
    KompotEngine
    ${SOURCES}
    ${HEADERS}
)

target_compile_definitions(
    KompotEngine PRIVATE
    VULKAN_HPP_NO_EXCEPTIONS=1
)
target_include_directories(
    KompotEngine
    PUBLIC "${CMAKE_SOURCE_DIR}/Source"
)
target_link_libraries(KompotEngine
        ${GLFW_LIBRARIES}
        ${PYTHON_LIBRARIES}
)
