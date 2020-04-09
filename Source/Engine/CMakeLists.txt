cmake_minimum_required(VERSION 3.12)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()


set(ENGINE_HEADERS
    EngineConfig.hpp
    Engine.hpp
    IEngineSystem.hpp
    ClientSubsystem/ClientSubsystem.hpp
    ClientSubsystem/Window/Window.hpp
)

set(ENGINE_SOURCES
    Engine.cpp
)

if (WIN32)
    list(APPEND ENGINE_SOURCES
        ClientSubsystem/ClientSubsystemWindows.cpp
        ClientSubsystem/Window/WindowWindows.cpp
        )
endif()

if (LINUX)
    list(APPEND ENGINE_SOURCES
        ClientSubsystem/ClientSubsystemLinux.cpp
        #ClientSubsystem/Window/WindowLinux.cpp
        )
endif()

add_library(Engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
    ${GLOBAL_INCLUDES}
)


target_link_libraries(Engine user32.lib)
target_compile_features(Engine PUBLIC cxx_std_17)
