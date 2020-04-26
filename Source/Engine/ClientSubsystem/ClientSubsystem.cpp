/*
*  ClientSubsystem.cpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#include "ClientSubsystem.hpp"

using namespace KompotEngine;

ClientSubsystem::ClientSubsystem(int argc, char** argv, const EngineConfig& engineConfig)
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
