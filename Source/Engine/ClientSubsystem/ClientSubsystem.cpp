/*
*  ClientSubsystem.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include "ClientSubsystem.hpp"
#include <Engine/Log/Log.hpp>

using namespace Kompot;

ClientSubsystem::ClientSubsystem()
{
    m_mainWindow = new Window("Game");
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
