cmake_minimum_required(VERSION 3.12)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()


set(ENGINE_HEADERS
    Log/Log.hpp
    DebugUtils/DebugUtils.hpp
    EngineConfig.hpp
    Engine.hpp
    Config/ConfigManager.hpp
    IEngineSystem.hpp
    ClientSubsystem/ClientSubsystem.hpp
    ClientSubsystem/Window/Window.hpp
    ClientSubsystem/Renderer/IRenderer.hpp
    ClientSubsystem/Renderer/Vulkan/VulkanRenderer.hpp
    ClientSubsystem/Renderer/Vulkan/VulkanDevice.hpp
    ClientSubsystem/Renderer/Vulkan/VulkanUtils.hpp
)

set(ENGINE_SOURCES
    Log/Log.cpp
    DebugUtils/DebugUtils.cpp
    Config/ConfigManager.cpp
    Engine.cpp
    ClientSubsystem/ClientSubsystem.cpp
    ClientSubsystem/Renderer/Vulkan/VulkanRenderer.cpp
    ClientSubsystem/Renderer/Vulkan/VulkanDevice.cpp
    ClientSubsystem/Renderer/Vulkan/VulkanUtils.cpp
)

if (WIN32)
    list(APPEND ENGINE_SOURCES
        ClientSubsystem/Window/WindowWindows.cpp
    )
endif()

if (LINUX)
    list(APPEND ENGINE_SOURCES
        ClientSubsystem/Window/WindowLinux.cpp
    )
endif()

add_library(Engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
    ${GLOBAL_INCLUDES}
)

if(LINUX)
    find_package(PkgConfig)
    pkg_check_modules(PKG_X11_XCB REQUIRED x11-xcb)
    target_link_libraries(Engine ${PKG_X11_XCB_LIBRARIES})
endif()

if (WIN32)
    target_link_libraries(Engine Dbghelp)
endif()

target_compile_features(Engine PUBLIC cxx_std_17)
