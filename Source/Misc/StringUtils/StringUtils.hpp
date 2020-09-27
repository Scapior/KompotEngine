/*
*  StringUtils.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#pragma once

#include "EngineDefines.hpp"
#include "EngineTypes.hpp"
#include "Misc/Templates/Functions.hpp"
#include <array>

namespace StringUtils
{
    template<
        typename charType = char,
        typename T,
        std::size_t hexLenght = sizeof(T*) << 1
        >
    auto hexFromPointer(
            const T * const pointer,
            StringCaseOption stringCaseOption = StringCaseOption::Lowercase,
            TrimOption trimOption = TrimOption::Trim)
    {
        const auto hexDigits = stringCaseOption == StringCaseOption::Lowercase ? "0123456789abcdef"s : "0123456789ABCDEF"s;
        charType buff[hexLenght + 1]{};

        auto bitIndex = (hexLenght - 1) * 4;
        if (trimOption == TrimOption::Trim)
        {
            for (; (reinterpret_cast<uint64_t>(pointer) >> bitIndex) == 0 && bitIndex < hexLenght * 4; bitIndex -= 4);
        }
        for (std::size_t i = 0; i < hexLenght && bitIndex < hexLenght * 4; ++i,  bitIndex -= 4)
        {
            buff[i] = hexDigits[0x0f & (reinterpret_cast<uint64_t>(pointer) >> bitIndex)];
        }
        if (buff[0] == 0)
        {
            buff[0] = '0';
        }
        return std::basic_string<charType>(buff);
    }

    // ToDo
    auto fromIntiger(int64_t value) // placeholder
    {
        return std::to_string(value);
    }
};

