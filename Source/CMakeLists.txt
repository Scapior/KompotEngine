cmake_minimum_required(VERSION 3.12)

set(GLOBAL_INCLUDE_PATH
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> #global.hpp
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

add_executable(
    KompotEngine WIN32
    global.hpp
    main.cpp
)

target_link_libraries(KompotEngine PRIVATE Engine Misc ${Vulkan_LIBRARY})
