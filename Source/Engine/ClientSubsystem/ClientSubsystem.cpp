/*
 *  ClientSubsystem.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "ClientSubsystem.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"
#include <Engine/Log/Log.hpp>

using namespace Kompot;
using namespace Kompot::Rendering::Vulkan;

ClientSubsystem::ClientSubsystem()
{
    mRenderer   = new VulkanRenderer();
    mMainWindow = new Window("Game", mRenderer);
}

ClientSubsystem::~ClientSubsystem()
{
    if (mMainWindow)
    {
        delete mMainWindow;
    }
    if (mRenderer)
    {
        delete mRenderer;
    }
}

void ClientSubsystem::run()
{
    mMainWindow->run();
}
