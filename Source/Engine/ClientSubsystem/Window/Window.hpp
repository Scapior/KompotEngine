/*
 *  Window.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once
#include "../Renderer/IRenderer.hpp"
#include <EngineDefines.hpp>
#include <EngineTypes.hpp>
#include <Misc/Templates/Functions.hpp>
#include <atomic>
#include <string>
#include <string_view>

namespace Kompot
{
struct PlatformHandlers;

class Window
{
public:
    Window(std::string_view windowName, Kompot::IRenderer* renderer = nullptr, const PlatformHandlers* parentWindowHandlers = nullptr);
    ~Window();

    void run();
    void closeWindow();

    WindowRendererAttributes* getWindowRendererAttributes() const
    {
        return mWindowRendererAttributes;
    }

    void setWindowRendererAttributes(WindowRendererAttributes* windowRendererAttributes)
    {
        mWindowRendererAttributes = windowRendererAttributes;
    }

private:
    std::string mWindowName;
    Kompot::IRenderer* mRenderer;

    PlatformHandlers* mWindowHandlers             = nullptr;
    const PlatformHandlers* mParentWindowHandlers = nullptr;

    WindowRendererAttributes* mWindowRendererAttributes = nullptr;

    /* Platform-specific definitions */

#ifdef ENGINE_OS_WINDOWS
    std::atomic_bool mNeedToClose        = false;
    static constexpr auto windowClassName = TemplateUtils::makeArray(L"KompotEngineWindow");
    static int64_t __stdcall windowProcedure(void* hwnd, uint32_t message, uint64_t wParam, int64_t lParam);
#endif
};

} // namespace Kompot
