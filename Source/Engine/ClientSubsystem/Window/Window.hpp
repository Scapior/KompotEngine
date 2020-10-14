/*
*  Window.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#pragma once
#include <EngineTypes.hpp>
#include <EngineDefines.hpp>
#include <atomic>
#include <string>
#include <string_view>
#include <Misc/Templates/Functions.hpp>

namespace Kompot
{

struct PlatformHandlers;

class Window
{
public:
    Window(std::string_view windowName, const PlatformHandlers* parentWindowHandlers = nullptr);
	~Window();

    void run();
    void closeWindow();
private:
    std::string      m_windowName;

    PlatformHandlers* m_windowHandlers = nullptr;
    const PlatformHandlers* m_parentWindowHandlers = nullptr;    

/* Platform-specific definitions */

#ifdef ENGINE_OS_WINDOWS
    std::atomic_bool m_needToClose = false;
    static constexpr auto windowClassName = TemplateUtils::makeArray(L"KompotEngineWindow");
#endif

#ifdef ENGINE_OS_WINDOWS_x64
    static int64_t _KompotEngine_stdcall windowProcedure(void* hwnd, uint32_t message, uint64_t wParam, int64_t lParam);
#endif

#ifdef ENGINE_OS_WINDOWS_x32
    static int32_t __stdcall windowProcedure(void* hwnd, uint32_t message, uint32_t wParam, int32_t lParam);
#endif

};

}
