/*
 *  Log.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "Log.hpp"
#include <string>
#include <ctime>


Log::Log()
{
    using namespace Kompot;
    m_logFile.open("log.txt");
    *this << '[' << timeNow() << ']' << " Log initialized" << std::endl;
}

Log& Log::operator<<(const std::chrono::system_clock::time_point& time)
{
    std::array<wchar_t, 100> buffer;
    const auto timeOldFormat = std::chrono::system_clock::to_time_t(time);
#ifdef ENGINE_COMPILER_MSVC
    std::tm timeStruct{};
    std::localtime_s(&timeStruct, &timeOldFormat);
    std::wcsftime(buffer.data(), buffer.size(), mDateTimeFormat, &timeStruct);
#else
    std::wcsftime(buffer.data(), buffer.size(), mDateTimeFormat, std::localtime(&timeOldFormat));
#endif
    return *this;
}

Log& Log::write(const char* text, std::streamsize size)
{
    m_logFile.write(text, size);
    return *this;
}
