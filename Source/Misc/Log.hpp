#pragma once

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

class Log
{
public:
    static Log& getInstance()
    {
        static Log logSingltone;
        return logSingltone;
    }

    static void callbackForGlfw(int, const char*);

    template <typename T>
    Log& operator<<(const T& value)
    {
        std::lock_guard<std::mutex> scopeLock(m_mutex);
        m_logFile << value;
        m_logFile.flush();
        return *this;
    }

    typedef std::ostream& (*ostream_manipulator)(std::ostream&);
    Log& operator<<(ostream_manipulator pf)
    {
        return operator<< <ostream_manipulator>(pf);
    }

    ~Log()
    {
        m_logFile.close();
    }

private:
    Log()
    {
        m_logFile.open("log.txt");
    }
    std::ofstream m_logFile;
    std::mutex    m_mutex;
};
