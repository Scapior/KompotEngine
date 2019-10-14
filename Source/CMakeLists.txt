cmake_minimum_required(VERSION 3.12)

set(SOURCES
    main.cpp
    Engine/Engine.cpp
    Misc/Log.cpp
    Misc/TickingObject.cpp
    Misc/OptionsParser/OptionsParser.cpp
)

set(HEADERS
    global.hpp
    Engine/Engine.hpp
    Engine/IEngineSystem.hpp
    Engine/Component.hpp
    Misc/Log.hpp
    Misc/TickingObject.hpp
    Misc/OptionsParser/OptionsParser.hpp
)

add_executable(
    KompotEngine
    ${SOURCES}
    ${HEADERS}
)

target_compile_definitions(
    KompotEngine PRIVATE
    VULKAN_HPP_NO_EXCEPTIONS=1
)

target_include_directories(
    KompotEngine
    PUBLIC "${CMAKE_SOURCE_DIR}/Source"
)

if (WIN32)
    target_link_libraries(KompotEngine
            vulkan-1
    )
else()
    target_link_libraries(KompotEngine
            vulkan
            stdc++fs
    )
endif()
