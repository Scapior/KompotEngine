/*
*  StringUtils_tests.cpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#include <Misc/StringUtils/StringUtils.hpp>
#include <gtest/gtest.h>


TEST(hexAddressFromPointer, lowerCase)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x1234567890abcdef);
    const std::string targetResult { "1234567890abcdef" };

    const auto result = StringUtils::hexFromPointer(deadcodePointer, StringCaseOption::Lowercase);
    EXPECT_EQ(result, targetResult) << "Result:" << result;
}

TEST(hexAddressFromPointer, upperCase)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x1234567890abcdef);
    const std::string targetResult { "1234567890ABCDEF" };

    const auto result = StringUtils::hexFromPointer(deadcodePointer, StringCaseOption::Uppercase);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, trim)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x0000000012345678);
    const std::string targetResult { "12345678" };

    const auto result = StringUtils::hexFromPointer(deadcodePointer, StringCaseOption::Uppercase, TrimOption::Trim);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, dontTrim)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x0000000012345678);
    const std::string targetResult { "0000000012345678" };

    const auto result = StringUtils::hexFromPointer(deadcodePointer, StringCaseOption::Uppercase, TrimOption::DontTrim);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, nullptrTrim)
{
    const void * const deadcodePointer = nullptr;
    const std::string targetResult { "0" };

    const auto result = StringUtils::hexFromPointer(deadcodePointer, StringCaseOption::Uppercase, TrimOption::Trim);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, nullptrDontTrim)
{
    const void * const deadcodePointer = nullptr;
    const std::string targetResult { "0000000000000000" };

    const auto result = StringUtils::hexFromPointer(deadcodePointer, StringCaseOption::Uppercase, TrimOption::DontTrim);
    EXPECT_EQ(result, targetResult);
}
