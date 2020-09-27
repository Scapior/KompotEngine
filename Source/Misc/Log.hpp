/*
*  Log.hpp
*  Copyright (C) 2020 by Maxim Stoianov
*  Licensed under the MIT license.
*/

#pragma once

#include <EngineTypes.hpp>
#include <EngineDefines.hpp>
#include "DateTimeFormatter.hpp"
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>

class Log
{
typedef std::ostream& (*OstreamManipulator)(std::ostream&);

public:
    static Log& getInstance()
    {
        static Log logSingltone;
        return logSingltone;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*);

    static void setupDebugCallback(VkInstance, VkDevice);
    static void deleteDebugCallback();

    inline static auto timeNow() { return std::chrono::system_clock::now(); }

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

    Log& operator<<(const KompotEngine::DateTimeFormat& dateTimeFormatter);

    Log& operator<<(const std::chrono::system_clock::time_point& time);

    ~Log()
    {
        m_logFile.close();
    }

    /* ostream-like part for templates*/
    char fill() const { return m_logFile.fill(); }
    char fill(char fillCharacter) { return m_logFile.fill(fillCharacter); }

    std::streamsize width() const { return m_logFile.width(); }
    std::streamsize width(std::streamsize newWidthValue) { return m_logFile.width(newWidthValue); }

    Log& write(const char* text, std::streamsize size);

    /* end the ostream-like part */

private:
    Log();

    std::ofstream m_logFile;
    std::mutex    m_mutex;

    KompotEngine::DateTimeFormatter m_dateTimeFormatter;

#ifdef ENGINE_DEBUG
    VkDebugUtilsMessengerEXT m_vkDebugMessenger;
    VkInstance m_vkDebugMessengerInstance;
    static PFN_vkCreateDebugUtilsMessengerEXT  pfn_vkCreateDebugUtilsMessengerEXT;
    static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT;
#endif
};
