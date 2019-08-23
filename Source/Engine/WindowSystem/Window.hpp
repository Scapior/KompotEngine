#pragma once

#include "global.hpp"

#ifdef ENGINE_PLATFORM_LINUX

#include "WindowXCB.hpp"

#elif define(ENGINE_PLATFORM_WIN32)

#include "WindowWin32.hpp"

#endif

