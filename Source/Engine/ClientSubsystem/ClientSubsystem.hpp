/*
 *  ClientSubsystem.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */


#pragma once

#include "Renderer/RenderingCommon.hpp"
#include "Window/Window.hpp"
#include <Engine/EngineConfig.hpp>
#include <Engine/IEngineSystem.hpp>
#include <EngineTypes.hpp>
//#include <condition_variable>
//#include <atomic>

//#ifdef ENGINE_OS_LINUX
//#include <xcb/xcb.h>
//#endif

#include <array>

namespace Kompot
{
class ClientSubsystem : public IEngineSystem
{
public:
    ClientSubsystem();
    ~ClientSubsystem();

    void run(/*std::condition_variable& conditionVariable*/) override;

private:
    /*std::atomic_flag*/ bool mNeedToExit = false;

public:
    bool isNeedToExit() const
    {
        return mNeedToExit;
    }

private:
    Window* mMainWindow  = nullptr;
    Kompot::Rendering::IRenderer* mRenderer = nullptr;
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
