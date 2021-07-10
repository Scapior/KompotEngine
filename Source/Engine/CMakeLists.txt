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
        ClientSubsystem/Renderer/Vulkan/VulkanPipelineBuilder.hpp
        Platform/MessageDialog.hpp)

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
        ClientSubsystem/Renderer/Vulkan/VulkanPipelineBuilder.cpp
        ClientSubsystem/Window/Window.cpp
        Platform/MessageDialog.cpp)

add_library(Engine STATIC
        ${ENGINE_SOURCES}
        ${ENGINE_HEADERS}
        ${GLOBAL_INCLUDES}
        )

option(ENGINE_USE_XCB_INSTEAD_XLIB "Use XCB library, not Xlib")

if (UNIX)
    if (ENGINE_USE_XCB_INSTEAD_XLIB)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(PKG_X11_XCB REQUIRED x11-xcb)
        add_compile_definitions(ENGINE_USE_XCB VK_USE_PLATFORM_XCB_KHR)
        target_link_libraries(Engine ${PKG_X11_XCB_LIBRARIES})
        message(FATAL_ERROR "XCB support not implemented yet")
    else()
        find_package(X11 REQUIRED)
        add_compile_definitions(ENGINE_USE_XLIB VK_USE_PLATFORM_XLIB_KHR)
        target_link_libraries(Engine ${X11_LIBRARIES})
    endif()
endif ()

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

if (WIN32)
    target_link_libraries(Engine Dbghelp)
endif ()

target_compile_features(Engine PUBLIC cxx_std_20)
