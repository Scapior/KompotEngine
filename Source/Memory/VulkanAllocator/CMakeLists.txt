cmake_minimum_required(VERSION 3.12)

execute_process(
  COMMAND git submodule update --remote --recursive
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_library(VulkanAllocator STATIC
        ${PROJECT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/src/vk_mem_alloc.natvis
        VulkanAllocator.cpp
        )

file(COPY ${PROJECT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/include/vk_mem_alloc.h
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

file(RENAME vk_mem_alloc.h VulkanAllocator.hpp)

target_compile_features(VulkanAllocator PUBLIC cxx_std_17)
