/*
*  StringUtils.hpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#pragma once

#include "EngineDefines.hpp"
#include "EngineTypes.hpp"
#include "Misc/Templates/Functions.hpp"
#include <array>

namespace StringUtils
{
    template<
        StringCaseOption stringCaseOption = StringCaseOption::Lowercase,
        TrimOption trimOption = TrimOption::Trim,
        typename charType = char,
        typename T,
        std::size_t hexLenght = sizeof(T*) << 1
        >
    constexpr auto hexAddressFromPointer(const T * const pointer)
    {
        constexpr auto hexDigits = stringCaseOption == StringCaseOption::Lowercase ? TemplateUtils::makeArray("0123456789abcdef") : TemplateUtils::makeArray("0123456789ABCDEF");
        charType buff[hexLenght + 1]{};

        auto bitIndex = (hexLenght - 1) * 4;
        if constexpr (trimOption == TrimOption::Trim)
        {
            for (; (reinterpret_cast<uint64_t>(pointer) >> bitIndex) == 0; bitIndex -= 4);
        }
        for (std::size_t i = 0; i < hexLenght && bitIndex < hexLenght * 4; ++i,  bitIndex -= 4)
        {
            buff[i] = hexDigits[0x0f & (reinterpret_cast<uint64_t>(pointer) >> bitIndex)];
        }
        return  std::basic_string<charType>(buff);
    }

    // ToDo
    auto fromIntiger(int64_t value) // placeholder
    {
        return std::to_string(value);
    }
};

