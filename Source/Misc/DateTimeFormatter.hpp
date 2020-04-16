/*
*  DateTimeFormatter.hpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#pragma once
#include <vector>
#include <chrono>
#include <ctime>
#include <ostream>
#include <iomanip>
#include "Templates/Functions.hpp"
#include <EngineDefines.hpp>

namespace KompotEngine
{

enum class DateTimeFormatSpecifier // also  check ISO 8601
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

struct DateTimeFormat
{
    std::vector<DateTimeFormatSpecifier> data;

    template<std::size_t N>
    DateTimeFormat(const std::array<DateTimeFormatSpecifier, N>& format)
    {
        data.reserve(N);
        data.insert(data.begin(), format.begin(), format.end());
    }

    DateTimeFormat& operator+(const DateTimeFormatSpecifier& format)
    {
        data.push_back(format);
        return *this;
    }
};

} // namespace KompotEngine

inline constexpr auto operator+(const KompotEngine::DateTimeFormatSpecifier valueLeft, const KompotEngine::DateTimeFormatSpecifier valueRight)
{
    return std::array<KompotEngine::DateTimeFormatSpecifier, 2> {valueLeft, valueRight};
}

inline auto operator+(const KompotEngine::DateTimeFormatSpecifier& formatSpecifier, KompotEngine::DateTimeFormat& dateTimeFormat)
{
    dateTimeFormat.data.insert(dateTimeFormat.data.begin(), formatSpecifier);
    return dateTimeFormat;
}

namespace KompotEngine
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
        localtime_s(&m_parsedTime, &timeValue);
        printTime(stream);
    }

    void setFormat(const DateTimeFormat& format) { m_dateTimeFormat = format; }
    const DateTimeFormat& getFormat() const { return m_dateTimeFormat; }

private:


    DateTimeFormat m_dateTimeFormat =
            DateTimeFormatSpecifier::Year       + DateTimeFormatSpecifier::Dot +
            DateTimeFormatSpecifier::Month      + DateTimeFormatSpecifier::Dot +
            DateTimeFormatSpecifier::Day        + DateTimeFormatSpecifier::Space +
            DateTimeFormatSpecifier::Hours24    + DateTimeFormatSpecifier::Colon +
            DateTimeFormatSpecifier::Minutes    + DateTimeFormatSpecifier::Colon +
            DateTimeFormatSpecifier::Seconds    + DateTimeFormatSpecifier::Dot +
            DateTimeFormatSpecifier::Milliseconds;

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
        for (const DateTimeFormatSpecifier& formatSpecifier : m_dateTimeFormat.data)
        {
            switch (formatSpecifier)
            {
            case DateTimeFormatSpecifier::Year:
            {
                stream << m_parsedTime.tm_year + 1900;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::Month:
                stream.width(2);
            case DateTimeFormatSpecifier::MonthShort:
            {
                stream << m_parsedTime.tm_mon + 1;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::Day:
                stream.width(2);
            case DateTimeFormatSpecifier::DayShort:
            {
                stream << m_parsedTime.tm_mday;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::Hours24:
                stream.width(2);
            case DateTimeFormatSpecifier::Hours24Short:
            {
                stream << m_parsedTime.tm_hour;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::Hours12:
                stream.width(2);
            case DateTimeFormatSpecifier::Hours12Short:
            {
                stream << getHoursInFormat12();
                stream.width(previousWidth);
                break;
            }



            case DateTimeFormatSpecifier::Minutes:
                stream.width(2);
            case DateTimeFormatSpecifier::MinutesShort:
            {
                stream << m_parsedTime.tm_min;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::Seconds:
                stream.width(2);
            case DateTimeFormatSpecifier::SecondsShort:
            {
                stream << m_parsedTime.tm_sec;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::Milliseconds:
                stream.width(3);
            case DateTimeFormatSpecifier::MillisecondsShort:
            {
                stream << m_parsedMilliseconds.count();
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::WeeklyName:
                stream << m_daysNames[m_parsedTime.tm_wday];
                break;
            case DateTimeFormatSpecifier::WeeklyNameShort:
                stream.write(m_daysNames[m_parsedTime.tm_wday], 3);
                break;

            case DateTimeFormatSpecifier::MonthName:
                stream << m_monthsNames[m_parsedTime.tm_mon];
                break;
            case DateTimeFormatSpecifier::MonthNameShort:
                stream.write(m_monthsNames[m_parsedTime.tm_mon], 3);
                break;

            case DateTimeFormatSpecifier::DayOfYear:
                stream.width(3);
            case DateTimeFormatSpecifier::DayOfYearShort:
            {
                stream << m_parsedTime.tm_yday + 1;
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::DayOfWeek:
                stream << (m_parsedTime.tm_yday != 0 ?  m_parsedTime.tm_wday : 7);
                break;
            case DateTimeFormatSpecifier::DayOfWeekFromNull:
                stream << (m_parsedTime.tm_yday != 0 ?  m_parsedTime.tm_wday - 1 : 6);
                break;

            case DateTimeFormatSpecifier::DayPartName:
                stream <<  (m_parsedTime.tm_hour < 12 ? "AM" : "PM");
                break;

            case DateTimeFormatSpecifier::WeekNumber:
                stream.width(2);
            case DateTimeFormatSpecifier::WeekNumberShort:
            {
                stream << getWeekNumber();
                stream.width(previousWidth);
                break;
            }

            case DateTimeFormatSpecifier::Dot:
                stream << '.';
                break;
            case DateTimeFormatSpecifier::Comma:
                stream << ',';
                break;
            case DateTimeFormatSpecifier::Colon:
                stream << ':';
                break;
            case DateTimeFormatSpecifier::Space:
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

} // namespace KompotEngine
