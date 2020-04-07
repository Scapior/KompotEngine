/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

//#ifdef ENGINE_PLATFORM_LINUX

#pragma once
#include <global.hpp>
#include <Engine/EngineConfig.hpp>
#include <Engine/IEngineSystem.hpp>
//#include <condition_variable>
//#include <atomic>

//#ifdef ENGINE_PLATFORM_LINUX
//#include <xcb/xcb.h>
//#endif

#include <array>
#include "Window/Window.hpp"


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

    void run(/*std::condition_variable& conditionVariable*/) override;

private:
    /*std::atomic_flag*/ bool m_needToExit = false;
public:
    bool isNeedToExit() const { return m_needToExit; }

private:
    Window* m_mainWindow = nullptr;

//#ifdef ENGINE_PLATFORM_LINUX
//    xcb_connection_t* m_xcbConnection;
//    xcb_screen_t* m_xcbScreen;
//    xcb_window_t m_xcbWindow;
//#endif

//#ifdef ENGINE_PLATFORM_WIN32

//#endif

};

} // namespace KompotEngine

//#endif
