/*
 *  IEngineSystem.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

namespace Kompot
{
class IEngineSystem
{
public:
    virtual void run() = 0;

    virtual ~IEngineSystem()
    {
    }
};
} // namespace Kompot
