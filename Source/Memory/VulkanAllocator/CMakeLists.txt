cmake_minimum_required(VERSION 3.12)

find_package(Git QUIET) # https://cliutils.gitlab.io/modern-cmake/chapters/projects/submodule.html
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(STATUS "VulkanMemoryAllocator submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

add_library(VulkanAllocator STATIC
        ${PROJECT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/src/vk_mem_alloc.natvis
        VulkanAllocator.cpp
        )

file(COPY ${PROJECT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/include/vk_mem_alloc.h
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

file(RENAME vk_mem_alloc.h VulkanAllocator.hpp)

target_compile_features(VulkanAllocator PUBLIC cxx_std_17)
