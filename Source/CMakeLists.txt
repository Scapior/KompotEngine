cmake_minimum_required(VERSION 3.12)

set(GLOBAL_INCLUDE_PATH
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # include <EngineDefines.hpp>, etc
        )

find_package(Vulkan REQUIRED)

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
