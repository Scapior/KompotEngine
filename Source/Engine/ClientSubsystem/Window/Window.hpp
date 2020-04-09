/*
*   Copyright (C) 2020 by Maxim Stoyanov
*
*   scapior.github.io
*/

#pragma once
#include <global.hpp>
#include <string>
#include <string_view>

namespace KompotEngine
{

struct PlatformHandlers;

class Window
{
public:
    Window(std::string_view windowName, const PlatformHandlers* parentWindowHandlers = nullptr);

    void run();
private:
    std::string      m_windowName;

    PlatformHandlers* m_windowHandlers = nullptr;
    const PlatformHandlers* m_parentWindowHandlers = nullptr;

// Platform-specific definitions
#ifdef ENGINE_PLATFORM_WIN32
    static int64_t __stdcall windowProcedure(void* hwnd, uint32_t message, uint64_t wParam, int64_t lParam);
#endif

};

}
