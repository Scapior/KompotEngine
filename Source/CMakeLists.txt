cmake_minimum_required(VERSION 3.12)

set(GLOBAL_INCLUDE_PATH
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # include <EngineDefines.hpp>, etc
        )

find_package(Vulkan REQUIRED)
find_package(GLM REQUIRED)

add_compile_definitions(VULKAN_HPP_ASSERT_ON_RESULT=static_cast<void>)

if (WIN32)
    add_compile_definitions(ENGINE_OS_WINDOWS)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
    add_compile_definitions(NOMINMAX) # to avoid winapi's trash macros in global space
endif ()

if (UNIX AND NOT APPLE)
    add_compile_definitions(ENGINE_OS_LINUX)
    add_compile_definitions(VK_USE_PLATFORM_XCB_KHR)
endif ()

add_subdirectory(Misc)
target_include_directories(Misc
        PRIVATE
        ${GLOBAL_INCLUDE_PATH}
        ${Vulkan_INCLUDE_DIR}
        )

add_subdirectory(Engine)
target_include_directories(Engine
        PUBLIC
        ${GLOBAL_INCLUDE_PATH}
        ${Vulkan_INCLUDE_DIR}
        ${GLM_INCLUDE_DIRS}
        )

add_subdirectory(Math)

add_executable(
        KompotEngine WIN32
        EngineDefines.hpp
        EngineTypes.hpp
        main.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_link_libraries(KompotEngine PRIVATE pthread)
endif ()

target_link_libraries(KompotEngine PRIVATE Engine Misc Math ${Vulkan_LIBRARY})
set_target_properties(KompotEngine
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
