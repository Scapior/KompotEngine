cmake_minimum_required(VERSION 3.12)

project(KompotEngine
        VERSION 0.0.0.1
        LANGUAGES CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GCC")
    list(APPEND CMAKE_CXX_FLAGS "-pthread -fno-exceptions ${CMAKE_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_CXX_FLAGS "/Od")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GCC")
        set(CMAKE_CXX_FLAGS "-O0")
    endif()
endif()


add_subdirectory(Source)
