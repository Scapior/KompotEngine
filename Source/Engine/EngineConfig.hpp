/*
*  EngineConfig.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#pragma once

#include <EngineTypes.hpp>

namespace KompotEngine
{

struct EngineConfig
{
    bool isEditMode;
    bool isFullscreen;
    uint16_t windowWidth;
    uint16_t windowHeight;
    bool isMaximized;
};

}
