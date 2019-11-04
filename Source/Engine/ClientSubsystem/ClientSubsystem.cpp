/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#include "ClientSubsystem.hpp"

using namespace KompotEngine;

ClientSubsystem::ClientSubsystem(int argc, char** argv, const EngineConfig& engineConfig)
{
#ifdef ENGINE_PLATFORM_LINUX    
    m_xcbConnection = xcb_connect(nullptr, nullptr);
    assert(m_xcbConnection);
    auto setup = xcb_get_setup(m_xcbConnection);
    auto xcbScreenIteratior = xcb_setup_roots_iterator(setup);
    xcb_screen_next(&xcbScreenIteratior);

#endif
}

ClientSubsystem::~ClientSubsystem()
{
#ifdef ENGINE_PLATFORM_LINUX
    xcb_disconnect(m_xcbConnection);
#endif
}
