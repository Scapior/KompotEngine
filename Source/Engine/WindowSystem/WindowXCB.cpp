#include "Window.hpp"

using namespace KompotEngine::WindowSystem;

Window::Window()
{
    m_connection = xcb_connect(nullptr, nullptr);


}

