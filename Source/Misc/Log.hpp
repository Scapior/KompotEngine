#pragma once

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

class Log
{
typedef std::ostream& (*OstreamManipulator)(std::ostream&);

public:
    static Log& getInstance()
    {
        static Log logSingltone;
        return logSingltone;
    }

    static void callbackForGlfw(int, const char*);
    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*);

    template <typename T>
    Log& operator<<(const T& value)
    {
        std::lock_guard<std::mutex> scopeLock(m_mutex);
        m_logFile << value;
        m_logFile.flush();
        return *this;
    }

    Log& operator<<(OstreamManipulator pf)
    {
        return operator<< <OstreamManipulator>(pf);
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
