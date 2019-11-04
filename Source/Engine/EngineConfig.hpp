/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#pragma once

#include <global.hpp>

namespace KompotEngine
{

struct EngineConfig
{
    bool isEditMode;
    bool isFullscreen;
    uint32_t windowWidth;
    uint32_t windowHeight;
    bool isMaximized;
};

}
