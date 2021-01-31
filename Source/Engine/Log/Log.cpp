/*
 *  Log.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "Log.hpp"
#include <string>

const Log::DateTimeBlock_t Log::DateTimeBlock{};

Log::Log()
{
    using namespace Kompot;
    m_logFile.open("log.txt");
    *this << DateTimeBlock << " Log initialized" << std::endl;
}

Log& Log::operator<<(const Kompot::DateTimeFormat& dateTimeFormat)
{
    m_dateTimeFormatter.setFormat(dateTimeFormat);
    return *this;
}

Log& Log::operator<<(const std::chrono::system_clock::time_point& time)
{
    m_dateTimeFormatter.printTime(*this, time);
    return *this;
}

Log& Log::write(const char* text, std::streamsize size)
{
    m_logFile.write(text, size);
    return *this;
}
Log& Log::operator<<(const Log::DateTimeBlock_t& value)
{
    *this << '[' << Log::timeNow() << ']';
    return *this;
}
