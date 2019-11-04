cmake_minimum_required(VERSION 3.12)

set(SOURCES
    main.cpp
    Engine/Engine.cpp
    Engine/ClientSubsystem/ClientSubsystem.hpp
    Misc/Log.cpp
    Misc/OptionsParser/OptionsParser.cpp
)

set(HEADERS
    global.hpp
    Engine/EngineConfig.hpp
    Engine/Engine.hpp
    Engine/IEngineSystem.hpp
    Engine/ClientSubsystem/ClientSubsystem.cpp
    Misc/Log.hpp
    Misc/OptionsParser/OptionsParser.hpp
)

add_executable(
    KompotEngine
    ${SOURCES}
    ${HEADERS}
)

include_directories(.)

target_compile_definitions(
    KompotEngine PRIVATE
    VULKAN_HPP_NO_EXCEPTIONS=1
)

if (WIN32)
    target_link_libraries(KompotEngine
            vulkan-1
    )
else()
    find_package(PkgConfig)
    pkg_check_modules(PKG_X11_XCB QUIET x11-xcb)
    target_link_libraries(KompotEngine
            vulkan
            stdc++fs
            ${PKG_X11_XCB_LIBRARIES}
    )
endif()
