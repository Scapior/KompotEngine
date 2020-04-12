/*
*  DateTimeStringFormatter.hpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#pragma once
#include <string>
#include <chrono>
#include <stack>
#include "Templates/Functions.hpp"

namespace KompotEngine
{

enum class DateTimeFormatSpecifier // also  check ISO 8601
{
    Year,           // Y
    YearShort,      //y
    Month,          // m
    Day,            // d
    DayShort,       //e
    Hours24,        // H
    Hours24Short,   // k GNU
    Hours12,        // I
    Hours12Short,   // l GNU
    Minutes,        // M
    Seconds,        // S
    Nanoseconds, // n -- custom!
    WeeklyName,         // %A
    WeeklyNameShort,    // %a
    MonthName,          // %B
    MonthNameShort,     // %b
    DayOfYear,          // %j 051, 251
    DayOfWeek,          // u 1-7
    DayOfWeekFromNull,  // w 0-6
    DayPartName,        // p -- AM/PM  GNU
    WeekNumber          // %V -- Week number
};

constexpr std::array<char, 2> convertDateTimeFormat(DateTimeFormatSpecifier Value)
{
    switch (Value)
    {
        case DateTimeFormatSpecifier::Year:
            return std::array<char, 2>{'%', 'Y'};
        case DateTimeFormatSpecifier::YearShort:
            return std::array<char, 2>{'%', 'y'};
        case DateTimeFormatSpecifier::Month:
            return std::array<char, 2>{'%', 'm'};
        case DateTimeFormatSpecifier::Day:
            return std::array<char, 2>{'%', 'd'};
        case DateTimeFormatSpecifier::DayShort:
            return std::array<char, 2>{'%', 'e'};
        case DateTimeFormatSpecifier::Hours24:
            return std::array<char, 2>{'%', 'H'};
        case DateTimeFormatSpecifier::Hours24Short:
            return std::array<char, 2>{'%', 'k'}; // (GNU)
        case DateTimeFormatSpecifier::Hours12:
            return std::array<char, 2>{'%', 'I'};
        case DateTimeFormatSpecifier::Hours12Short:
            return std::array<char, 2>{'%', 'l'}; // (GNU)
        case DateTimeFormatSpecifier::Minutes:
            return std::array<char, 2>{'%', 'M'};
        case DateTimeFormatSpecifier::Seconds:
            return std::array<char, 2>{'%', 'S'};
        case DateTimeFormatSpecifier::Nanoseconds:
            return std::array<char, 2>{'%', 'n'};
        case DateTimeFormatSpecifier::WeeklyName:
            return std::array<char, 2>{'%', 'A'};
        case DateTimeFormatSpecifier::WeeklyNameShort:
            return std::array<char, 2>{'%', 'a'};
        case DateTimeFormatSpecifier::MonthName:
            return std::array<char, 2>{'%', 'B'};
        case DateTimeFormatSpecifier::MonthNameShort:
            return std::array<char, 2>{'%', 'b'};
        case DateTimeFormatSpecifier::DayOfYear:
            return std::array<char, 2>{'%', 'j'};
        case DateTimeFormatSpecifier::DayOfWeek:
            return std::array<char, 2>{'%', 'u'}; // 1-7
        case DateTimeFormatSpecifier::DayOfWeekFromNull:
            return std::array<char, 2>{'%', 'w'}; // 0-6
        case DateTimeFormatSpecifier::DayPartName:
            return std::array<char, 2>{'%', 'p'}; // AM/PM  (GNU)
        case DateTimeFormatSpecifier::WeekNumber:
            return std::array<char, 2>{'%', 'V'};
    };
    return {};
}

template<std::size_t N>
struct DateTimeFormat
{
    constexpr DateTimeFormat(const std::array<char, N>& array)
        : data(array) {}

    constexpr DateTimeFormat(const char& value)
        : DateTimeFormat(std::array<char, 1>{value}) {}

    constexpr DateTimeFormat(const DateTimeFormatSpecifier& value)
        : DateTimeFormat(convertDateTimeFormat(value)) {}

    constexpr auto operator+(const DateTimeFormatSpecifier& value)
    {
        return DateTimeFormat<N+2>(data + convertDateTimeFormat(value));
    }

    constexpr auto operator+(const char character)
    {
        return DateTimeFormat<N+1>(data + character);
    }

    constexpr std::string_view getString() const
    {
        return std::string_view(data.data(), data.size());
    }

    std::array<char, N> data;
};

} // namespace KompotEngine

inline constexpr auto operator+(const KompotEngine::DateTimeFormatSpecifier valueLeft, const KompotEngine::DateTimeFormatSpecifier valueRight)
{
    return KompotEngine::DateTimeFormat<2>(valueLeft) + valueRight;
}

inline constexpr auto operator+(const char character, const KompotEngine::DateTimeFormatSpecifier formatValue)
{
    return KompotEngine::DateTimeFormat<3>(character + KompotEngine::convertDateTimeFormat(formatValue));
}

inline constexpr KompotEngine::DateTimeFormat<3> operator+(const KompotEngine::DateTimeFormatSpecifier formatValue, const char character)
{
    return KompotEngine::DateTimeFormat<2>(formatValue) + character;
}

template<std::size_t N>
inline constexpr auto operator+(const char character, const KompotEngine::DateTimeFormat<N>& format)
{
    return KompotEngine::DateTimeFormat<N+1>(character + format.data);
}

namespace KompotEngine
{

class DateTimeStringFormatter
{
public:
    static constexpr auto defaultFormat =
            DateTimeFormatSpecifier::Year + '.' +
            DateTimeFormatSpecifier::Month + '.' +
            DateTimeFormatSpecifier::Day + ' ' +
            DateTimeFormatSpecifier::Hours24 + ':' +
            DateTimeFormatSpecifier::Minutes + ':' +
            DateTimeFormatSpecifier::Seconds + '.' +
            DateTimeFormatSpecifier::Nanoseconds;

    static constexpr auto logDateTimeFormat = '[' + defaultFormat + ']';

    static std::string now();

    template<std::size_t N>
    static std::string now(const DateTimeFormat<N>& format)
    {
        const auto time = std::chrono::high_resolution_clock::now();
        return toString(format, time);
    }

    template<std::size_t N, typename Clock, typename Duration>
    static std::string toString(const DateTimeFormat<N>& format, const std::chrono::time_point<Clock, Duration>& time)
    {
        (void)time;
        std::string result{};
        result = format.getString();
        return result;
    }

};

} // namespace KompotEngine
