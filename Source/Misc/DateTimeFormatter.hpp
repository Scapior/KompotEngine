/*
*  DateTimeFormatter.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#pragma once
#define __STDC_WANT_LIB_EXT1__
#include <vector>
#include <chrono>
#include <ctime>
#include <ostream>
#include <iomanip>
#include "Templates/Functions.hpp"
#include <EngineDefines.hpp>

namespace Kompot
{

struct DateTimeFormat
{
    enum FormatSpecifier // also  check ISO 8601
    {
        Year,
        Month,
        MonthShort,
        Day,
        DayShort,
        Hours24,
        Hours24Short,
        Hours12,
        Hours12Short,
        Minutes,
        MinutesShort,
        Seconds,
        SecondsShort,
        Milliseconds,
        MillisecondsShort,
        WeeklyName,
        WeeklyNameShort,
        MonthName,
        MonthNameShort,
        DayOfYear,
        DayOfYearShort,
        DayOfWeek,
        DayOfWeekFromNull,
        DayPartName,
        WeekNumber,
        WeekNumberShort,

        Dot,
        Comma,
        Colon,
        Space
    };
    std::vector<FormatSpecifier> data;

    template<std::size_t N>
    DateTimeFormat(const std::array<FormatSpecifier, N>& format)
    {
        data.reserve(N);
        data.insert(data.begin(), format.begin(), format.end());
    }

    DateTimeFormat& operator+(const FormatSpecifier& format)
    {
        data.push_back(format);
        return *this;
    }
};

} // namespace Kompot

inline constexpr auto operator+(const Kompot::DateTimeFormat::FormatSpecifier valueLeft, const Kompot::DateTimeFormat::FormatSpecifier valueRight)
{
    return std::array<Kompot::DateTimeFormat::FormatSpecifier, 2> {valueLeft, valueRight};
}

inline auto operator+(const Kompot::DateTimeFormat::FormatSpecifier& formatSpecifier, Kompot::DateTimeFormat& dateTimeFormat)
{
    dateTimeFormat.data.insert(dateTimeFormat.data.begin(), formatSpecifier);
    return dateTimeFormat;
}

namespace Kompot
{

class DateTimeFormatter
{
public:
    template<class T>
    void printTime(T& stream, const std::chrono::system_clock::time_point& timePoint) const
    {
        using Ms = std::chrono::milliseconds;
        using Sec = std::chrono::seconds;
        using Clock = std::chrono::system_clock;

        const auto timeValue = Clock::to_time_t(timePoint);
        const auto timeValueMs = std::chrono::duration_cast<Ms>(timePoint.time_since_epoch());
        const auto timeValueSec = Sec(timeValue);
        m_parsedMilliseconds = timeValueMs - std::chrono::duration_cast<Ms>(timeValueSec);
#if defined(ENGINE_OS_WINDOWS)
        localtime_s(&m_parsedTime, &timeValue); // MSVS version
#elif defined(ENGINE_OS_LINUX)
        localtime_r(&timeValue, &m_parsedTime);
        //gmtime_r(&timeValue, &m_parsedTime);
#endif
        printTime(stream);
    }

    void setFormat(const DateTimeFormat& format) { m_dateTimeFormat = format; }
    const DateTimeFormat& getFormat() const { return m_dateTimeFormat; }

private:


    DateTimeFormat m_dateTimeFormat =
            DateTimeFormat::Year       + DateTimeFormat::Dot +
            DateTimeFormat::Month      + DateTimeFormat::Dot +
            DateTimeFormat::Day        + DateTimeFormat::Space +
            DateTimeFormat::Hours24    + DateTimeFormat::Colon +
            DateTimeFormat::Minutes    + DateTimeFormat::Colon +
            DateTimeFormat::Seconds    + DateTimeFormat::Dot +
            DateTimeFormat::Milliseconds;

    mutable std::tm m_parsedTime;
    mutable std::chrono::milliseconds m_parsedMilliseconds;



    static constexpr std::array<const char*, 7> m_daysNames = {
        "Sunday", // tm::tm_wday with value 0 means Sunday. Fuck the Bible for this
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };

    static constexpr std::array<const char*, 12> m_monthsNames = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };
    template<class T>
    void printTime(T& stream) const
    {
        const char previousCharacter = stream.fill();
        stream.fill('0');
        std::streamsize previousWidth = stream.width();
        for (const DateTimeFormat::FormatSpecifier& formatSpecifier : m_dateTimeFormat.data)
        {
            switch (formatSpecifier)
            {
            case DateTimeFormat::Year:
            {
                stream << m_parsedTime.tm_year + 1900;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Month:
                stream.width(2);
            case DateTimeFormat::MonthShort:
            {
                stream << m_parsedTime.tm_mon + 1;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Day:
                stream.width(2);
            case DateTimeFormat::DayShort:
            {
                stream << m_parsedTime.tm_mday;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Hours24:
                stream.width(2);
            case DateTimeFormat::Hours24Short:
            {
                stream << m_parsedTime.tm_hour;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Hours12:
                stream.width(2);
            case DateTimeFormat::Hours12Short:
            {
                stream << getHoursInFormat12();
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Minutes:
                stream.width(2);
            case DateTimeFormat::MinutesShort:
            {
                stream << m_parsedTime.tm_min;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Seconds:
                stream.width(2);
            case DateTimeFormat::SecondsShort:
            {
                stream << m_parsedTime.tm_sec;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Milliseconds:
                stream.width(3);
            case DateTimeFormat::MillisecondsShort:
            {
                stream << m_parsedMilliseconds.count();
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::WeeklyName:
                stream << m_daysNames[m_parsedTime.tm_wday];
                break;
            case DateTimeFormat::WeeklyNameShort:
                stream.write(m_daysNames[m_parsedTime.tm_wday], 3);
                break;

            case DateTimeFormat::MonthName:
                stream << m_monthsNames[m_parsedTime.tm_mon];
                break;
            case DateTimeFormat::MonthNameShort:
                stream.write(m_monthsNames[m_parsedTime.tm_mon], 3);
                break;

            case DateTimeFormat::DayOfYear:
                stream.width(3);
            case DateTimeFormat::DayOfYearShort:
            {
                stream << m_parsedTime.tm_yday + 1;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::DayOfWeek:
                stream << (m_parsedTime.tm_yday != 0 ?  m_parsedTime.tm_wday : 7);
                break;
            case DateTimeFormat::DayOfWeekFromNull:
                stream << (m_parsedTime.tm_yday != 0 ?  m_parsedTime.tm_wday - 1 : 6);
                break;

            case DateTimeFormat::DayPartName:
                stream <<  (m_parsedTime.tm_hour < 12 ? "AM" : "PM");
                break;

            case DateTimeFormat::WeekNumber:
                stream.width(2);
            case DateTimeFormat::WeekNumberShort:
            {
                stream << getWeekNumber();
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormat::Dot:
                stream << '.';
                break;
            case DateTimeFormat::Comma:
                stream << ',';
                break;
            case DateTimeFormat::Colon:
                stream << ':';
                break;
            case DateTimeFormat::Space:
                stream << ' ';
                break;
            default:
                breakPoint("Unknown enum value");
                break;
            }
        }
        stream.fill(previousCharacter);
    }

    int getWeekNumber() const
    {
        constexpr int daysPerWeek = 7;

        const int wday = m_parsedTime.tm_wday ;
        const int delta = wday ? wday-1 : daysPerWeek-1 ;
        return ((m_parsedTime.tm_yday + daysPerWeek - delta) / daysPerWeek)+1;
    }

    int getHoursInFormat12() const
    {
        if (m_parsedTime.tm_hour == 0)
        {
            return 12;
        }
        if (m_parsedTime.tm_hour > 12)
        {
            return m_parsedTime.tm_hour - 12;
        }
        return m_parsedTime.tm_hour;
    }
};

} // namespace Kompot
