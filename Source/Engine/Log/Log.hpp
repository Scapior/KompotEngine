/*
 *  Log.hpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#pragma once

#include <EngineDefines.hpp>
#include <EngineTypes.hpp>
#include <Misc/DateTimeFormatter.hpp>
#include <vulkan/vulkan.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <iostream>

class Log
{
    typedef std::ostream& (*OstreamManipulator)(std::ostream&);

public:
    static Log& getInstance()
    {
        static Log logSingltone;
        return logSingltone;
    }

    inline static auto timeNow()
    {
        return std::chrono::system_clock::now();
    }

    template<typename T>
    Log& operator<<(const T& value)
    {
        std::lock_guard<std::mutex> scopeLock(m_mutex);
        m_logFile << value << std::flush;
#if defined(ENGINE_DEBUG)
        std::cout << value << std::flush;
#endif
        return *this;
    }

    Log& operator<<(OstreamManipulator pf)
    {
        return operator<<<OstreamManipulator>(pf);
    }

    template<Kompot::DateTime::Specifier... specifiers>
    Log& operator<<(const Kompot::DateTime::DateTimeFormat<specifiers...>& dateTimeFormat)
    {
        mDateTimeFormat = dateTimeFormat.get();
        return *this;
    }

    Log& operator<<(const std::chrono::system_clock::time_point& time);

    ~Log()
    {
        m_logFile.close();
    }

    /* ostream-like part for templates*/
    char fill() const
    {
        return m_logFile.fill();
    }
    char fill(char fillCharacter)
    {
        return m_logFile.fill(fillCharacter);
    }

    std::streamsize width() const
    {
        return m_logFile.width();
    }
    std::streamsize width(std::streamsize newWidthValue)
    {
        return m_logFile.width(newWidthValue);
    }

    Log& write(const char* text, std::streamsize size);

    /* end the ostream-like part */

private:
    Log();

    char* const mDateTimeFormat = Kompot::DateTime::DateTimeFormat<Kompot::DateTime::Specifier::LogDateTime>().get();

    std::ofstream m_logFile;
    std::mutex m_mutex;
};
