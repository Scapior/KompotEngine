#pragma once

#include "global.hpp"
#include <xcb/xcb.h>

namespace KompotEngine
{

namespace WindowSystem
{

class Window
{
public:
    Window();
private:
    xcb_connection_t* m_connection;
    xcb_window_t m_window;
    xcb_screen_t* m_screen;
    xcb_atom_t m_wmProtocols;
    xcb_atom_t m_wmDeleteWin;

};

} // namespace WindowSystem

} // namespace KompotEngine
