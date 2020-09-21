/*
*   StringUtils_tests.cpp
*   21.9.2020
*   Copyright (C) 2020 by Maxim Stoyanov
*
*   scapior.github.io
*/

#include <Misc/StringUtils/StringUtils.hpp>
#include <gtest/gtest.h>


TEST(hexAddressFromPointer, lowerCase)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x1234567890abcdef);
    const std::string targetResult { "1234567890abcdef" };

    const auto result = StringUtils::hexAddressFromPointer<StringCaseOption::Lowercase>(deadcodePointer);
    EXPECT_EQ(result, targetResult) << "Result:" << result;
}

TEST(hexAddressFromPointer, upperCase)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x1234567890abcdef);
    const std::string targetResult { "1234567890ABCDEF" };

    const auto result = StringUtils::hexAddressFromPointer<StringCaseOption::Uppercase>(deadcodePointer);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, trim)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x0000000012345678);
    const std::string targetResult { "12345678" };

    const auto result = StringUtils::hexAddressFromPointer<StringCaseOption::Uppercase, TrimOption::Trim>(deadcodePointer);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, dontTrim)
{
    const void * const deadcodePointer = reinterpret_cast<void*>(0x0000000012345678);
    const std::string targetResult { "0000000012345678" };

    const auto result = StringUtils::hexAddressFromPointer<StringCaseOption::Uppercase, TrimOption::DontTrim>(deadcodePointer);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, nullptrTrim)
{
    const void * const deadcodePointer = nullptr;
    const std::string targetResult { "0" };

    const auto result = StringUtils::hexAddressFromPointer<StringCaseOption::Uppercase, TrimOption::Trim>(deadcodePointer);
    EXPECT_EQ(result, targetResult);
}

TEST(hexAddressFromPointer, nullptrDontTrim)
{
    const void * const deadcodePointer = nullptr;
    const std::string targetResult { "0000000000000000" };

    const auto result = StringUtils::hexAddressFromPointer<StringCaseOption::Uppercase, TrimOption::DontTrim>(deadcodePointer);
    EXPECT_EQ(result, targetResult);
}
