/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#pragma once
#include <global.hpp>
#include <Engine/EngineConfig.hpp>
#include <Engine/IEngineSystem.hpp>

#ifdef ENGINE_PLATFORM_LINUX
#include <xcb/xcb.h>
#endif

/*!
    \class ClientSubsystem
    \brief This class contain platform depend code for client case.

    This class controlling a window managment, rendering, player input, audio.
*/

namespace KompotEngine
{

class ClientSubsystem : public IEngineSystem
{
public:
    ClientSubsystem(int argc, char** argv, const EngineConfig& engineConfig);
    ~ClientSubsystem();
private:
#ifdef ENGINE_PLATFORM_LINUX
    xcb_connection_t* m_xcbConnection;
    xcb_screen_t* m_xcbScreenInformation;
#endif

#ifdef ENGINE_PLATFORM_WIN32

#endif

};

} // namespace KompotEngine
