/*
 *  Vector.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

namespace Math
{
template<typename T = float>
class Vector2D
{
public:
    T x, y;

    T foo()
    {
        return x + y;
    }
};

template<typename T = float>
class Vector
{
public:
    T x, y, z;

    T foo()
    {
        return x + y + z;
    }
};

} // namespace Math
