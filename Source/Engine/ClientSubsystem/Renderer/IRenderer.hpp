/*
 *  IRenderer.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

namespace Kompot
{
class Window;

struct WindowRendererAttributes
{
    virtual ~WindowRendererAttributes()
    {
    }
};

class IRenderer
{
public:
    virtual WindowRendererAttributes* updateWindowAttributes(Window* window) = 0;
    virtual void unregisterWindow(Window* window)                            = 0;
};

} // namespace Kompot
