/*
 *  DateTimeFormatter.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once
#define __STDC_WANT_LIB_EXT1__
#include "Templates/Functions.hpp"
#include <EngineDefines.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <vector>
#include <array>

namespace Kompot::DateTime
{
enum Specifier
{
    Year,             /** Year as 1997, 2021 */
    YearShort,        /** In range [00, 99] (e.g. 97 for 1997) */
    Month,            /** 01, 09, 12 */
    MonthName,        /** E.g. "October", depends from locale */
    MonthNameShort,   /** "Oct" for October, depends from locale */
    WeekOfYear,       /** ISO 8601 week of the year (range [01,53]) */
    DayOfYear,        /** Day of the year (range [001,366]) */
    DayOfMonth,       /** Day of the month (range [01,31]) */
    DayOfMonthShort,  /** Day of the month (range [1,31]). Single digit is preceded by a space */
    Weekday,          /** Monday is 1 (ISO 8601 format) (range [1-7]) */
    WeekdayName,      /** Weekday name, e.g. Friday (locale dependent) */
    WeekdayNameShort, /** Short weekday name, e.g. "Fri" for Friday (locale dependent) */
    Hour,             /** 24 hour clock (range [00-23]) */
    HourShort,        /** 12 hour clock (range [01-12) */
    Minute,           /** Range [00,59] */
    Second,           /** Range [00,59] */
    DateTime,         /** E.g. "Sun Oct 17 04:41:13 2010" (locale dependent) */
    DayPeriod,        /** Localized a.m. or p.m. (locale dependent) */
    TimeISO,          /** "%H:%M:%S" (ISO 8601) */
    DateISO,          /** "%Y-%m-%d" (ISO 8601) */
    LogDateTime,      /** "%Y.%m.%d %H:%M:S" */
    Dot,              /** '.' */
    Comma,            /** ',' */
    Slash,            /** '/' */
    Colon,            /** ':' */
    Hyphen,           /** '-' */
    Space             /** ' ' */
};

template<Specifier... specifiers>
class DateTimeFormat
{
public:
    constexpr auto get()
    {
        return format.data();
    }

private:
    static constexpr const auto format = getFormat<specifiers...>();

    template<Specifier First, Specifier... letters>
    constexpr const auto getFormat()
    {
        return Kompot::TemplateUtils::concatArrays(ImplGetFormat<First>(), ImplGetFormat<letters...>());
    }

    template<>
    constexpr const auto getFormat<Specifier::Year>()
    {
        return std::to_array("%Y");
    }

    template<>
    constexpr const auto getFormat<Specifier::YearShort>()
    {
        return std::to_array("%y");
    }

    template<>
    constexpr const auto getFormat<Specifier::Month>()
    {
        return std::to_array("%m");
    }

    template<>
    constexpr const auto getFormat<Specifier::MonthName>()
    {
        return std::to_array("%B");
    }

    template<>
    constexpr const auto getFormat<Specifier::MonthNameShort>()
    {
        return std::to_array("%b");
    }

    template<>
    constexpr const auto getFormat<Specifier::WeekOfYear>()
    {
        return std::to_array("%V");
    }

    template<>
    constexpr const auto getFormat<Specifier::DayOfYear>()
    {
        return std::to_array("%j");
    }

    template<>
    constexpr const auto getFormat<Specifier::DayOfMonth>()
    {
        return std::to_array("%d");
    }

    template<>
    constexpr const auto getFormat<Specifier::DayOfMonthShort>()
    {
        return std::to_array("%e");
    }

    template<>
    constexpr const auto getFormat<Specifier::Weekday>()
    {
        return std::to_array("%u");
    }

    template<>
    constexpr const auto getFormat<Specifier::WeekdayName>()
    {
        return std::to_array("%A");
    }

    template<>
    constexpr const auto getFormat<Specifier::WeekdayNameShort>()
    {
        return std::to_array("%a");
    }

    template<>
    constexpr const auto getFormat<Specifier::Hour>()
    {
        return std::to_array("%H");
    }

    template<>
    constexpr const auto getFormat<Specifier::HourShort>()
    {
        return std::to_array("%I");
    }

    template<>
    constexpr const auto getFormat<Specifier::Minute>()
    {
        return std::to_array("%M");
    }

    template<>
    constexpr const auto getFormat<Specifier::Second>()
    {
        return std::to_array("%S");
    }

    template<>
    constexpr const auto getFormat<Specifier::DateTime>()
    {
        return std::to_array("%c");
    }

    template<>
    constexpr const auto getFormat<Specifier::DayPeriod>()
    {
        return std::to_array("%p");
    }

    template<>
    constexpr const auto getFormat<Specifier::TimeISO>()
    {
        return std::to_array("%T");
    }

    template<>
    constexpr const auto getFormat<Specifier::DateISO>()
    {
        return std::to_array("%F");
    }

    template<>
    constexpr const auto getFormat<Specifier::LogDateTime>()
    {
        return std::to_array("%Y.%m.%d %H:%M:S");
    }

    template<>
    constexpr const auto getFormat<Specifier::Dot>()
    {
        return std::to_array(".");
    }

    template<>
    constexpr const auto getFormat<Specifier::Comma>()
    {
        return std::to_array(",");
    }

    template<>
    constexpr const auto getFormat<Specifier::Slash>()
    {
        return std::to_array("/");
    }

    template<>
    constexpr const auto getFormat<Specifier::Colon>()
    {
        return std::to_array(":");
    }

    template<>
    constexpr const auto getFormat<Specifier::Hyphen>()
    {
        return std::to_array("-");
    }

    template<>
    constexpr const auto getFormat<Specifier::Space>()
    {
        return std::to_array(" ");
    }
};

} // namespace Kompot::DateTime
