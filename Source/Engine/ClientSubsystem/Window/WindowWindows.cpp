/*
 *  WindowWindows.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "Window.hpp"
#include "Windows.h" // winapi
#include <Engine/DebugUtils/DebugUtils.hpp>
#include <Engine/ErrorHandling.hpp>
#include <Engine/ClientSubsystem/Renderer/Vulkan/VulkanRenderer.hpp>
#include <Engine/Log/Log.hpp>

using namespace Kompot;

struct Kompot::PlatformHandlers
{
    HINSTANCE instanceHandler;
    HWND windowHandler;

    wchar_t* windowNameWideCharBuffer;
    std::size_t windowNameWideCharBufferSize;
};

Window::Window(std::string_view windowName, Kompot::IRenderer* renderer, const PlatformHandlers* parentWindowHandlers) :
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
        mWidth,
        mHeight,
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
    VulkanRenderer* vulkanRenderer = dynamic_cast<VulkanRenderer*>(mRenderer);
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
