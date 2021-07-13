/*
 *  RenderingCommon.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include <string_view>

namespace Kompot
{
class Window;
}

namespace Kompot::Rendering
{

struct WindowRendererAttributes
{
    virtual ~WindowRendererAttributes()
    {
    }
};

enum class ShaderType
{
    Vertex,
    Fragment,
    Compute
};

class IShader
{
public:
    virtual ~IShader(){};
    virtual std::string_view getSourceFilename() const = 0;
};

class IRenderer
{
public:
    virtual ~IRenderer(){};

    virtual void draw(Window* window) = 0;

    virtual void notifyWindowResized(Window* window)                         = 0;
    virtual WindowRendererAttributes* updateWindowAttributes(Window* window) = 0;
    virtual void unregisterWindow(Window* window)                            = 0;
    virtual std::string_view getName() const                                 = 0;
};

} // namespace Kompot
