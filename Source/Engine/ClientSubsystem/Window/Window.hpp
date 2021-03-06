/*
 *  Window.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once
#include "../Renderer/RenderingCommon.hpp"
#include <EngineDefines.hpp>
#include <EngineTypes.hpp>
#include <Misc/Templates/Functions.hpp>
#include <atomic>
#include <string>
#include <string_view>
#include <vulkan/vulkan.hpp>

namespace Kompot
{
struct PlatformHandlers;

class Window
{
public:
    Window(std::string_view windowName, Kompot::Rendering::IRenderer* renderer = nullptr, const PlatformHandlers* parentWindowHandlers = nullptr);
    ~Window();

    void run();
    void closeWindow();

    std::array<uint32_t, 2> getExtent() const;

    Kompot::Rendering::WindowRendererAttributes* getWindowRendererAttributes() const
    {
        return mWindowRendererAttributes;
    }

    vk::SurfaceKHR createVulkanSurface() const;

    void setWindowRendererAttributes(Kompot::Rendering::WindowRendererAttributes* windowRendererAttributes)
    {
        mWindowRendererAttributes = windowRendererAttributes;
    }

private:
    std::string mWindowName;
    Kompot::Rendering::IRenderer* mRenderer;

    std::atomic_bool  mNeedToClose                = false;
    PlatformHandlers* mWindowHandlers             = nullptr;
    const PlatformHandlers* mParentWindowHandlers = nullptr;

    Kompot::Rendering::WindowRendererAttributes* mWindowRendererAttributes = nullptr;

    /* Platform-specific definitions */

#ifdef ENGINE_OS_WINDOWS
    static const std::wstring windowClassNamePrefix;
    static int64_t __stdcall windowProcedure(void* hwnd, uint32_t message, uint64_t wParam, int64_t lParam);
#endif
};

} // namespace Kompot
