/*
*  ClientSubsystem.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "ClientSubsystem.hpp"
#include <Engine/Log/Log.hpp>
#include "Renderer/IRenderer.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

using namespace Kompot;

ClientSubsystem::ClientSubsystem()
{
    IRenderer* renderer = new VulkanRenderer();
    m_mainWindow = new Window("Game", renderer);
}

ClientSubsystem::~ClientSubsystem()
{
    delete m_mainWindow;
}
#include <thread>
void ClientSubsystem::run()
{
    m_mainWindow->run();
}
