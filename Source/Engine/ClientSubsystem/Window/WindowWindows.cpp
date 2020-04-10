#include "Window.hpp"
#include "Windows.h" // winapi

using namespace KompotEngine;

struct KompotEngine::PlatformHandlers
{
    HINSTANCE   instanceHandler;
    HWND        windowHandler;

    wchar_t*    windowNameWideCharBuffer;
    std::size_t windowNameWideCharBufferSize;
};

Window::Window(std::string_view windowName, const PlatformHandlers* parentWindowHandlers)
    : m_windowName(windowName),
      m_parentWindowHandlers(parentWindowHandlers)
{
    static const HINSTANCE currentAppHandlerInstance = GetModuleHandle(nullptr);

    m_windowHandlers = new PlatformHandlers{};

    m_windowHandlers->windowNameWideCharBufferSize = windowName.size() * 4; // 2?
    m_windowHandlers->windowNameWideCharBuffer = new wchar_t[m_windowHandlers->windowNameWideCharBufferSize]{};
    MultiByteToWideChar( CP_UTF8,
                         0_u32t,
                         windowName.data(), windowName.size(),
                         m_windowHandlers->windowNameWideCharBuffer,
                         static_cast<int>(m_windowHandlers->windowNameWideCharBufferSize)
    );       

    WNDCLASSEXW windowClass{};
    windowClass.cbSize        = sizeof(WNDCLASSEXW);
    windowClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc   = (WNDPROC)Window::windowProcedure;
    windowClass.cbClsExtra    = 0;
    windowClass.cbWndExtra    = sizeof(Window*); //to store poinetr
    windowClass.hInstance     = currentAppHandlerInstance;
    windowClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    windowClass.lpszMenuName  = NULL;
    windowClass.lpszClassName = windowClassName.data();
    windowClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&windowClass))
    {
        //std::terminate();
    }

    constexpr uint32_t windowStyleFlags =
            WS_SYSMENU |
            WS_BORDER |
            WS_CAPTION |
            WS_MAXIMIZEBOX |
            WS_MINIMIZEBOX |
            WS_SIZEBOX;

    m_windowHandlers->windowHandler = CreateWindowExW(
        0_u32t,
        windowClassName.data(),
        m_windowHandlers->windowNameWideCharBuffer,
        windowStyleFlags,
        CW_USEDEFAULT, // X
        CW_USEDEFAULT, // Y
        300,
        300,
        nullptr,
        nullptr,
        currentAppHandlerInstance,
        this
    );

    SetWindowLongPtr(m_windowHandlers->windowHandler, 0, reinterpret_cast<LONG_PTR>(this));

    if (!m_windowHandlers->windowHandler)
    {
//        DWORD errorMessageID = ::GetLastError();
//        if(errorMessageID == 0) return;

//        LPSTR messageBuffer = nullptr;
//        std::size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//                         NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

//        std::string message(messageBuffer, size);
//        Log::getInstance() << message << std::endl;
//        //Free the buffer.
//        LocalFree(messageBuffer);
//        std::terminate();
    }

}

void Window::run()
{
    ShowWindow(m_windowHandlers->windowHandler, SW_SHOW);
    MSG msg{};
    int iGetOk = 0;
    while ((iGetOk = GetMessage(&msg, NULL, 0, 0 )) != 0)
    {
        if (iGetOk == -1) return;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

#if defined (ENGINE_OS_WINDOWS_x32)
int32_t Window::windowProcedure(void* hwnd, uint32_t message, uint32_t wParam, int32_t lParam)
#elif defined(ENGINE_OS_WINDOWS_x64)
int64_t Window::windowProcedure(void* hwnd, uint32_t message, uint64_t wParam, int64_t lParam)
#endif
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, 0));
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
