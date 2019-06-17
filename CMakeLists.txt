cmake_minimum_required(VERSION 3.12)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)

if (WIN32)
    list(APPEND CMAKE_CXX_FLAGS "/EHsc /std:c++17")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${CMAKE_BINARY_DIR}/../)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../)
else()
    list(APPEND CMAKE_CXX_FLAGS "-pthread -fno-exceptions -std=c++17 ${CMAKE_CXX_FLAGS}")
endif()

project(KompotEngine)

set(SOURCES
    Source/main.cpp
    Source/Engine/Engine.cpp
    Source/Engine/PythonModule/PythonModule.cpp
    Source/Engine/PythonModule/PythonObject.cpp
    Source/Engine/World.cpp
    Source/Engine/MeshObject.cpp
    Source/Engine/Renderer/Device.cpp
    Source/Engine/Renderer/Renderer.cpp
    Source/Engine/Renderer/Shader.cpp
    Source/Engine/Renderer/ResourcesMaker.cpp
    Source/Engine/Renderer/Image.cpp
    Source/Engine/Renderer/Mesh.cpp
    Source/Engine/Renderer/Buffer.cpp
    Source/Engine/Renderer/SingleTimeCommandBuffer.cpp
    Source/Engine/Renderer/DescriptorPoolManager.cpp
    Source/Engine/IO/ResourcesLoader.cpp
    Source/Engine/IO/MeshesLoader.cpp
    Source/Engine/IO/TgaLoader.cpp
    Source/Misc/Log.cpp
    Source/Misc/TickingObject.cpp
    Source/Misc/OptionsParser/OptionsParser.cpp
)

set(HEADERS
    Source/global.hpp
    Source/Engine/Engine.hpp
    Source/Engine/PythonModule/PythonModule.hpp
    Source/Engine/PythonModule/PythonObject.hpp
    Source/Engine/World.hpp
    Source/Engine/MeshObject.hpp
    Source/Engine/Renderer/Device.hpp
    Source/Engine/Renderer/Renderer.hpp
    Source/Engine/Renderer/Shader.hpp
    Source/Engine/Renderer/ResourcesMaker.hpp
    Source/Engine/Renderer/Image.hpp
    Source/Engine/Renderer/Mesh.hpp
    Source/Engine/Renderer/Buffer.hpp
    Source/Engine/Renderer/SingleTimeCommandBuffer.hpp
    Source/Engine/Renderer/DescriptorPoolManager.hpp
    Source/Engine/IO/ResourcesLoader.hpp
    Source/Engine/IO/MeshesLoader.hpp
    Source/Engine/IO/TgaLoader.hpp
    Source/Misc/Log.hpp
    Source/Misc/TickingObject.hpp
    Source/Misc/OptionsParser/OptionsParser.hpp
)

add_subdirectory(Shaders)

if (WIN32)
    find_package(PythonLibs REQUIRED)
    include_directories(
            ${PYTHON_INCLUDE_DIRS}
            "$ENV{VULKAN_SDK}\\Include"
            "C:\\cpp\\Include"
    )
    link_directories(
        ${PYTHON_LIBRARIES}
        "$ENV{VULKAN_SDK}\\lib"
        "C:\\cpp\\Libs"
    )
endif()

if (UNIX)
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
endif()

add_executable(
    KompotEngine
    ${SOURCES}
    ${HEADERS}
    ${GLSL_SOURCE_FILES}
)

add_dependencies(KompotEngine Shaders)

target_compile_definitions(
    KompotEngine PRIVATE
    VULKAN_HPP_NO_EXCEPTIONS=1
)
target_include_directories(
    KompotEngine
    PUBLIC "${CMAKE_SOURCE_DIR}/Source"
)

if (WIN32)
    target_link_libraries(KompotEngine
            ${PYTHON_LIBRARIES}
            vulkan-1
            glfw3
    )
    file(COPY "C:\\cpp\\Libs\\glfw3.dll"
         DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
    target_link_libraries(KompotEngine
            ${GLFW_LIBRARIES}
            ${PYTHON_LIBRARIES}
            vulkan
            stdc++fs
    )
endif()
