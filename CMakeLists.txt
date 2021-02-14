cmake_minimum_required(VERSION 3.12)

project(KompotEngine
        VERSION 0.0.0.1
        LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_definitions(-DVULKAN_HPP_NO_EXCEPTIONS)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    add_definitions(-D_HAS_EXCEPTIONS=0)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Od")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -rdynamic")
    endif()
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX flags: ${CMAKE_CXX_FLAGS}")

add_subdirectory(Source)
add_subdirectory(Tests)
