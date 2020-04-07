#include "Window.hpp"
#include "Windows.h" // winapi

using namespace KompotEngine;

struct KompotEngine::PlatformHandlers
{
    HINSTANCE   instanceHandler;
    HWND        windowHandler;
};

Window::Window(std::string_view windowName, const PlatformHandlers* parentWindowHandlers)
    : m_windowName(windowName),
      m_parentWindowHandlers(parentWindowHandlers)
{
    std::size_t windowNameWideCharBufferSize = windowName.size() * 2;
    wchar_t* windowNameWideCharBuffer = new wchar_t[windowNameWideCharBufferSize];
    MultiByteToWideChar( CP_UTF8,
                             0_u32t,
                             windowName.data(), -1_32t,
                             windowNameWideCharBuffer,
                             static_cast<int>(windowNameWideCharBufferSize)
    );

//    CreateWindowExW(
//        _In_ DWORD dwExStyle,
//        _In_opt_ LPCWSTR lpClassName,
//        _In_opt_ LPCWSTR lpWindowName,
//        _In_ DWORD dwStyle,
//        _In_ int X,
//        _In_ int Y,
//        _In_ int nWidth,
//        _In_ int nHeight,
//        _In_opt_ HWND hWndParent,
//        _In_opt_ HMENU hMenu,
//        _In_opt_ HINSTANCE hInstance,
//        _In_opt_ LPVOID lpParam);

//    m_platformHandlers.windowHandler = CreateWindowEx(
//        0,
//        windowNameWideCharBuffer,
//        windowNameWideCharBuffer,
//                                           );
}

void Window::run()
{

}

void Window::show()
{

}
