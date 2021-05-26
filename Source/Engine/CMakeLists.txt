cmake_minimum_required(VERSION 3.14)

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif ()

set(ENGINE_HEADERS
        Engine.hpp
        ErrorHandling.hpp
        EngineConfig.hpp
        IEngineSystem.hpp
        Log/Log.hpp
        Config/ConfigManager.hpp
        DebugUtils/DebugUtils.hpp
        ClientSubsystem/ClientSubsystem.hpp
        ClientSubsystem/Window/Window.hpp
        ClientSubsystem/Renderer/IRenderer.hpp
        ClientSubsystem/Renderer/Vulkan/VulkanRenderer.hpp
        ClientSubsystem/Renderer/Vulkan/VulkanDevice.hpp
        ClientSubsystem/Renderer/Vulkan/VulkanUtils.hpp
        ClientSubsystem/Renderer/Vulkan/VulkanTypes.hpp
        ClientSubsystem/Renderer/Vulkan/VulkanShader.hpp
        ClientSubsystem/Renderer/Vulkan/VulkanPipelineBuilder.hpp)

set(ENGINE_SOURCES
        Engine.cpp
        ErrorHandling.cpp
        Log/Log.cpp
        Config/ConfigManager.cpp
        DebugUtils/DebugUtils.cpp
        ClientSubsystem/ClientSubsystem.cpp
        ClientSubsystem/Renderer/Vulkan/VulkanRenderer.cpp
        ClientSubsystem/Renderer/Vulkan/VulkanDevice.cpp
        ClientSubsystem/Renderer/Vulkan/VulkanUtils.cpp
        ClientSubsystem/Renderer/Vulkan/VulkanShader.cpp
        ClientSubsystem/Renderer/Vulkan/VulkanPipelineBuilder.cpp)

if (WIN32)
list(APPEND ENGINE_SOURCES
        ClientSubsystem/Window/WindowWindows.cpp
        )
endif ()

if (LINUX)
list(APPEND ENGINE_SOURCES
        ClientSubsystem/Window/WindowLinux.cpp
            )
endif ()

add_library(Engine STATIC
        ${ENGINE_SOURCES}
        ${ENGINE_HEADERS}
        ${GLOBAL_INCLUDES}
        )

if (LINUX)
    find_package(PkgConfig)
    pkg_check_modules(PKG_X11_XCB REQUIRED x11-xcb)
    target_link_libraries(Engine ${PKG_X11_XCB_LIBRARIES})
endif ()

if (WIN32)
    target_link_libraries(Engine Dbghelp)
endif ()

target_compile_features(Engine PUBLIC cxx_std_20)
