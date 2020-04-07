/*
*   Copyright (C) 2020 by Maxim Stoyanov
*
*   scapior.github.io
*/

#pragma once
#include <global.hpp>
#include <string>
#include <string_view>

namespace KompotEngine
{

struct PlatformHandlers;

class Window
{
public:
    Window(std::string_view windowName, const PlatformHandlers* parentWindowHandlers = nullptr);

    void run();

    void show();

private:
    std::string      m_windowName;

    PlatformHandlers* m_windowHandlers = nullptr;
    const PlatformHandlers* m_parentWindowHandlers = nullptr;
};

}
