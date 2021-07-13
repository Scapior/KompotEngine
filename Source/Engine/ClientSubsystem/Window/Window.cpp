/*
 *  WindowWindows.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "Window.hpp"
#include <Engine/DebugUtils/DebugUtils.hpp>
#include <Engine/ErrorHandling.hpp>
#include <Engine/ClientSubsystem/Renderer/RenderingCommon.hpp>
#include <Engine/ClientSubsystem/Renderer/Vulkan/VulkanRenderer.hpp>
#include <Engine/Log/Log.hpp>


#ifdef ENGINE_OS_WINDOWS

    #include "Windows.h" // winapi


using namespace Kompot;
using namespace Kompot::Rendering;
const std::wstring Window::windowClassNamePrefix = L"KompotEngineWindow_";

struct Kompot::PlatformHandlers
{
    HINSTANCE instanceHandler;
    HWND windowHandler;

    wchar_t* windowNameWideCharBuffer;
    std::size_t windowNameWideCharBufferSize;
};

Window::Window(std::string_view windowName, IRenderer* renderer, const PlatformHandlers* parentWindowHandlers) :
    mWindowName(windowName), mRenderer(renderer), mParentWindowHandlers(parentWindowHandlers)
{
    static const HINSTANCE currentAppHandlerInstance = ::GetModuleHandle(nullptr);

    mWindowHandlers = new PlatformHandlers{};

    mWindowHandlers->windowNameWideCharBufferSize = windowName.size() * 4; // 2?
    mWindowHandlers->windowNameWideCharBuffer     = new wchar_t[mWindowHandlers->windowNameWideCharBufferSize]{};

    check(::MultiByteToWideChar(
        CP_UTF8,
        0_u32t,
        windowName.data(),
        static_cast<int>(windowName.size()),
        mWindowHandlers->windowNameWideCharBuffer,
        static_cast<int>(mWindowHandlers->windowNameWideCharBufferSize)));

    const auto windowClassName = windowClassNamePrefix + mWindowHandlers->windowNameWideCharBuffer;

    WNDCLASSEXW windowClass{};
    windowClass.cbSize        = sizeof(WNDCLASSEXW);
    windowClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc   = (WNDPROC)Window::windowProcedure;
    windowClass.cbClsExtra    = 0;
    windowClass.cbWndExtra    = sizeof(Window*); // to store poinetr
    windowClass.hInstance     = currentAppHandlerInstance;
    windowClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName  = NULL;
    windowClass.lpszClassName = windowClassName.data();
    windowClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!::RegisterClassExW(&windowClass))
    {
        Kompot::ErrorHandling::exit("Failed to register window class, result code \"" + std::to_string(::GetLastError()) + "\"");
    }

    constexpr uint32_t windowStyleFlags = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX;

    mWindowHandlers->windowHandler = ::CreateWindowExW(
        0_u32t,
        windowClassName.data(),
        mWindowHandlers->windowNameWideCharBuffer,
        windowStyleFlags,
        CW_USEDEFAULT, // X
        CW_USEDEFAULT, // Y
        384,
        361,
        nullptr,
        nullptr,
        currentAppHandlerInstance,
        this);

    if (!mWindowHandlers->windowHandler)
    {
        Kompot::ErrorHandling::exit("Failed to create window, result code \"" + std::to_string(::GetLastError()) + "\"");
    }
    ::SetWindowLongPtr(mWindowHandlers->windowHandler, 0, reinterpret_cast<LONG_PTR>(this));

    mWindowRendererAttributes = renderer->updateWindowAttributes(this);
}

Window::~Window()
{
    if (mRenderer)
    {
        mRenderer->unregisterWindow(this);
    }
    if (mWindowRendererAttributes)
    {
        delete mWindowRendererAttributes;
        mWindowRendererAttributes = nullptr;
    }
    if (mWindowHandlers)
    {
        //        xcb_destroy_window(mWindowHandlers->xcbConnection, mWindowHandlers->xcbWindow);
        //        xcb_flush(mWindowHandlers->xcbConnection);
        //        // cleanup screen_t ?
        //        xcb_disconnect(mWindowHandlers->xcbConnection);
        //
        //        mWindowHandlers->xcbWindow = 0;
        //        mWindowHandlers->xcbScreen = nullptr;
        //        mWindowHandlers->xcbConnection = nullptr;
    }
    delete mWindowHandlers;
    mWindowHandlers = nullptr;
}

void Window::run()
{
    ::ShowWindow(mWindowHandlers->windowHandler, SW_SHOW);
    MSG msg{};
    int iGetOk = 0;
    while ((iGetOk = ::GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (iGetOk == -1)
            return;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

vk::SurfaceKHR Window::createVulkanSurface() const
{
    auto vulkanRenderer = dynamic_cast<Vulkan::VulkanRenderer*>(mRenderer);
    if (!vulkanRenderer)
    {
        check(!mRenderer) if (mRenderer)
        {
            Log::getInstance() << "Trying to create VkSurface with non-Vulkan renderer (" << mRenderer->getName() << ')' << std::endl;
        }
        else
        {
            Log::getInstance() << "Trying to create VkSurface with not setted renderer" << std::endl;
        }
        return nullptr;
    }

    const auto surfaceInfo = vk::Win32SurfaceCreateInfoKHR{}.setHinstance(::GetModuleHandle(nullptr)).setHwnd(mWindowHandlers->windowHandler);
    if (const auto createSurfaceResult = vulkanRenderer->getVkInstance().createWin32SurfaceKHR(surfaceInfo);
        createSurfaceResult.result == vk::Result::eSuccess)
    {
        return createSurfaceResult.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create VkSurface, result code \"" + vk::to_string(createSurfaceResult.result) + "\"");
    }

    return nullptr;
}

void Window::closeWindow()
{
    mNeedToClose = true;
}

int64_t Window::windowProcedure(void* hwnd, uint32_t message, uint64_t wParam, int64_t lParam)
{
    HWND hWnd      = reinterpret_cast<HWND>(hwnd);
    Window* window = reinterpret_cast<Window*>(::GetWindowLongPtr(hWnd, 0));

    if (!window)
    {
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }

    if (window->mNeedToClose)
    {
        ::PostQuitMessage(0);
        // DestroyWindow(hWnd);
    }

    switch (message)
    {
    // case WM_DESTROY: // https://stackoverflow.com/a/3155883
    case WM_PAINT:
    {
        if (window->mRenderer)
        {
            window->mRenderer->draw(window);
        }
        break;
    }
    case WM_SIZE:
        if (window->mRenderer)
        {
            window->mRenderer->notifyWindowResized(window);
        }
        break;
    case WM_CLOSE:
        ::PostQuitMessage(0);
        break;
    // case WM_SYSCHAR:
    // case WM_SYSCOMMAND: //A window receives this message when the user chooses a command from the
    // Window menu (formerly known as the system or control menu) or when the user chooses the
    // maximize button, minimize button, restore button, or close button. case WM_SETCURSOR: //Sent
    // to a window if the mouse causes the cursor to move within a window and mouse input is not
    // captured. case WM_DEVICECHANGE: // Notifies an application of a change to the hardware
    // configuration of a device or the computer.
    default:
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

std::array<uint32_t, 2> Window::getExtent() const
{
    std::array<uint32_t, 2> result{};

    RECT windowRectangle{};
    if (::GetWindowRect(mWindowHandlers->windowHandler, &windowRectangle))
    {
        result = {
            static_cast<uint32_t>(windowRectangle.right - windowRectangle.left),
            static_cast<uint32_t>(windowRectangle.bottom - windowRectangle.top)};
    }

    return result;
}

#endif // Windows

#if defined(ENGINE_OS_UNIX) and defined(ENGINE_USE_XLIB)
    #include <X11/Xlib.h>
    #include <sys/select.h>

using namespace Kompot::Rendering;

struct Kompot::PlatformHandlers
{
    // xcb_connection_t* xcbConnection;
    Display* xlibDisplay;
    int32_t xlibScreenNumber;
    ::Window xlibWindow;
    int32_t xlibConnectionDescriptor;

    uint32_t width;
    uint32_t height;
};

Kompot::Window::Window(std::string_view windowName, Kompot::Rendering::IRenderer* renderer, const PlatformHandlers* parentWindowHandlers) :
    mWindowName(windowName), mRenderer(renderer), mParentWindowHandlers(parentWindowHandlers)
{
    Log& log                     = Log::getInstance();
    mWindowHandlers              = new Kompot::PlatformHandlers{};
    mWindowHandlers->xlibDisplay = XOpenDisplay(nullptr);
    assert(mWindowHandlers->xlibDisplay);
    mWindowHandlers->xlibScreenNumber = DefaultScreen(mWindowHandlers->xlibDisplay);

    const ::Window rootWindow   = XDefaultRootWindow(mWindowHandlers->xlibDisplay);
    mWindowHandlers->xlibWindow = XCreateSimpleWindow(
        mWindowHandlers->xlibDisplay,
        rootWindow,
        /* x, y */ 0,
        0,
        /* w, h */ 300,
        300,
        1,
        XWhitePixel(mWindowHandlers->xlibDisplay, mWindowHandlers->xlibScreenNumber),
        XBlackPixel(mWindowHandlers->xlibDisplay, mWindowHandlers->xlibScreenNumber));
    //    mWindowHandlers->xlibWindow = XCreateWindow(
    //                mWindowHandlers->xlibDisplay, rootWindow,
    //                /* x, y */ 0, 0,
    //                /* w, h */ 300, 300,
    //                /* border */ 1

    //                );

    XSelectInput(
        mWindowHandlers->xlibDisplay,
        mWindowHandlers->xlibWindow,
        ExposureMask | StructureNotifyMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(mWindowHandlers->xlibDisplay, mWindowHandlers->xlibWindow);
    XFlush(mWindowHandlers->xlibDisplay);
    mWindowHandlers->xlibConnectionDescriptor = XConnectionNumber(mWindowHandlers->xlibDisplay);

    mWindowRendererAttributes = renderer->updateWindowAttributes(this);
    const auto extent         = getExtent();
    mWindowHandlers->width    = extent[0];
    mWindowHandlers->height   = extent[1];
}

Kompot::Window::~Window()
{
    if (mRenderer)
    {
        mRenderer->unregisterWindow(this);
    }
    if (mWindowRendererAttributes)
    {
        delete mWindowRendererAttributes;
        mWindowRendererAttributes = nullptr;
    }
    if (mWindowHandlers)
    {
        XDestroyWindow(mWindowHandlers->xlibDisplay, mWindowHandlers->xlibWindow);
        XCloseDisplay(mWindowHandlers->xlibDisplay);
    }
    delete mWindowHandlers;
    mWindowHandlers = nullptr;
}

void Kompot::Window::run()
{
    ::Atom xlibDeleteWindowMessageAtom = XInternAtom(mWindowHandlers->xlibDisplay, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(mWindowHandlers->xlibDisplay, mWindowHandlers->xlibWindow, &xlibDeleteWindowMessageAtom, 1);

    Log& log = Log::getInstance();
    XEvent xlibEvent;
    while (!mNeedToClose)
    {
        if (XPending(mWindowHandlers->xlibDisplay))
        {
            XNextEvent(mWindowHandlers->xlibDisplay, &xlibEvent);

            if (xlibEvent.type == ClientMessage && static_cast<Atom>(xlibEvent.xclient.data.l[0]) == xlibDeleteWindowMessageAtom)
            {
                mNeedToClose = true;
            }
        }

        if (mRenderer)
        {
            mRenderer->draw(this);
        }
    }
    // conditionVariable.wait()
}

void Kompot::Window::closeWindow()
{
    mNeedToClose = true;
}

vk::SurfaceKHR Kompot::Window::createVulkanSurface() const
{
    Rendering::Vulkan::VulkanRenderer* vulkanRenderer = dynamic_cast<Rendering::Vulkan::VulkanRenderer*>(mRenderer);
    if (!vulkanRenderer)
    {
        check(!mRenderer);
        if (mRenderer)
        {
            Log::getInstance() << "Trying to create VkSurface with non-Vulkan renderer (" << mRenderer->getName() << ')' << std::endl;
        }
        else
        {
            Log::getInstance() << "Trying to create VkSurface with not setted renderer" << std::endl;
        }
        return nullptr;
    }

    const auto surfaceInfo = vk::XlibSurfaceCreateInfoKHR{}.setDpy(mWindowHandlers->xlibDisplay).setWindow(mWindowHandlers->xlibWindow);
    if (const auto createSurfaceResult = vulkanRenderer->getVkInstance().createXlibSurfaceKHR(surfaceInfo);
        createSurfaceResult.result == vk::Result::eSuccess)
    {
        return createSurfaceResult.value;
    }
    else
    {
        Kompot::ErrorHandling::exit("Failed to create VkSurface, result code \"" + vk::to_string(createSurfaceResult.result) + "\"");
    }

    return nullptr;
}
std::array<uint32_t, 2> Kompot::Window::getExtent() const
{
    std::array<uint32_t, 2> result{};
    if (mWindowHandlers)
    {
        return {mWindowHandlers->width, mWindowHandlers->height};
    }
    return {};
}
#endif // XLIB
