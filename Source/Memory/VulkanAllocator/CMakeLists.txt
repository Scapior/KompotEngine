cmake_minimum_required(VERSION 3.12)

add_library(VulkanAllocator STATIC
        ${PROJECT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/src/vk_mem_alloc.natvis
        VulkanAllocator.cpp
        )

file(COPY ${PROJECT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/src/vk_mem_alloc.h
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

file(RENAME vk_mem_alloc.h VulkanAllocator.hpp)

target_compile_features(VulkanAllocator PUBLIC cxx_std_17)
