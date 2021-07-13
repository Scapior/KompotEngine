cmake_minimum_required(VERSION 3.14)

set(GLOBAL_INCLUDE_PATH
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # include <EngineDefines.hpp>, etc
        )

find_package(Vulkan REQUIRED)
add_compile_definitions(VULKAN_HPP_ASSERT_ON_RESULT=static_cast<void>)

if (WIN32)
    add_compile_definitions(ENGINE_OS_WINDOWS)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
    add_compile_definitions(NOMINMAX) # to avoid winapi's trash macros in global space
endif ()

if (UNIX AND NOT APPLE)
    add_compile_definitions(ENGINE_OS_UNIX)
    if (UNIX AND NOT APPLE)
        add_compile_definitions(ENGINE_OS_LINUX)
    endif()
endif()

add_subdirectory(Memory/VulkanAllocator)
target_include_directories(VulkanAllocator
        PRIVATE
        ${GLOBAL_INCLUDE_PATH}
        ${Vulkan_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/src/
        )

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
        main.cpp)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_link_libraries(KompotEngine PRIVATE pthread)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set_target_properties(KompotEngine PROPERTIES
            PDB_NAME "KompotEngine"
            PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    set_target_properties(Engine PROPERTIES
            PDB_NAME "Engine"
            PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    set_target_properties(Misc PROPERTIES
            PDB_NAME "Misc"
            PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    set_target_properties(Math PROPERTIES
            PDB_NAME "Math"
            PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endif ()

get_filename_component(VULKAN_SDK_LIBS_PATH ${Vulkan_LIBRARY} DIRECTORY)
target_link_directories(KompotEngine PRIVATE ${VULKAN_SDK_LIBS_PATH})

target_link_libraries(KompotEngine PRIVATE
        Engine
        Misc
        Math
        VulkanAllocator
        ${Vulkan_LIBRARY}
        glslang
        shaderc_combined
        SPIRV
        )

set_target_properties(KompotEngine
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
