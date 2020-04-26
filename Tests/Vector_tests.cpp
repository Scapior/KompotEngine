/*
*  Vector_tests.cpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#include <iostream>
#include <Math/Vector.hpp>
#include <gtest/gtest.h>


TEST(Vector2D, foo)
{
    Math::Vector2D v;
    v.x = 5;
    v.y = 3;
    EXPECT_FLOAT_EQ(v.foo(), 8.0f);
}

TEST(Vector, foo)
{
    Math::Vector v;
    v.x = 5;
    v.y = 3;
    v.z = 1;
    EXPECT_FLOAT_EQ(v.foo(), 9.0f);
}
