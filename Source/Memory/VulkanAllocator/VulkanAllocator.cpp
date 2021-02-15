/*
 *  VulkanAllocator.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#if !(__has_include("VulkanAllocator.hpp"))
static_assert(
    false,
    "Required VulkanMemoryAllocator from git submodule. "
    "Run: \"git submodule update --init --recursive\""
    " and after that rerun CMake.");
#endif

#define VMA_IMPLEMENTATION
#include "VulkanAllocator.hpp"




