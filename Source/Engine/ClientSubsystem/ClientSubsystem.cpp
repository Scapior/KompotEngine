/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#include "ClientSubsystem.hpp"
#include <unistd.h>

using namespace KompotEngine;

ClientSubsystem::ClientSubsystem(int argc, char** argv, const EngineConfig& engineConfig)
{
    Log& log = Log::getInstance();
#ifdef ENGINE_PLATFORM_LINUX    
    m_xcbConnection = xcb_connect(nullptr, nullptr);
    assert(m_xcbConnection);
    auto setup = xcb_get_setup(m_xcbConnection);
    auto xcbScreenIteratior = xcb_setup_roots_iterator(setup);
    for (uint32_t i = 1; xcbScreenIteratior.rem; xcb_screen_next(&xcbScreenIteratior), ++i)
    {
        m_xcbScreen = xcbScreenIteratior.data;
        log << "Founded screen #" << i << " (" << m_xcbScreen->width_in_pixels << '*' << m_xcbScreen->height_in_pixels << ")" << std::endl;
    }
    m_xcbWindow = xcb_generate_id(m_xcbConnection);
    xcb_void_cookie_t cookie = xcb_create_window(
                                   m_xcbConnection,
                                   XCB_COPY_FROM_PARENT,
                                   m_xcbWindow,
                                   m_xcbScreen->root,
                                   0, 0,
                                   engineConfig.windowWidth,
                                   engineConfig.windowHeight,
                                   10,
                                   XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                   m_xcbScreen->root_visual,
                                   0,
                                   nullptr);

    xcb_map_window(m_xcbConnection, m_xcbWindow);
    xcb_flush(m_xcbConnection);
    pause();
#endif
}

ClientSubsystem::~ClientSubsystem()
{
#ifdef ENGINE_PLATFORM_LINUX
    xcb_disconnect(m_xcbConnection);
#endif
}
