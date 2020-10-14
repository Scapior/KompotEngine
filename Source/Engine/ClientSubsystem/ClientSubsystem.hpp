/*
*  ClientSubsystem.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

//#ifdef ENGINE_OS_LINUX

#pragma once
#include <EngineTypes.hpp>
#include <Engine/EngineConfig.hpp>
#include <Engine/IEngineSystem.hpp>
//#include <condition_variable>
//#include <atomic>

//#ifdef ENGINE_OS_LINUX
//#include <xcb/xcb.h>
//#endif

#include <array>
#include "Window/Window.hpp"

namespace Kompot
{

class ClientSubsystem : public IEngineSystem
{
public:
    ClientSubsystem();
    ~ClientSubsystem();

    void run(/*std::condition_variable& conditionVariable*/) override;

private:
    /*std::atomic_flag*/ bool m_needToExit = false;
public:
    bool isNeedToExit() const { return m_needToExit; }

private:
    Window* m_mainWindow = nullptr;

//#ifdef ENGINE_OS_LINUX
//    xcb_connection_t* m_xcbConnection;
//    xcb_screen_t* m_xcbScreen;
//    xcb_window_t m_xcbWindow;
//#endif

//#ifdef ENGINE_OS_WIN32

//#endif

};

} // namespace Kompot

//#endif
