/*
 *  Log.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "Log.hpp"
#include <string>
#include "Misc/Templates/Functions.hpp"

#ifdef ENGINE_DEBUG
PFN_vkCreateDebugUtilsMessengerEXT Log::pfn_vkCreateDebugUtilsMessengerEXT   = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT Log::pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;
#endif

Log::Log()
{
    using namespace Kompot;
    m_logFile.open("log.txt");
    *this << '[' << Log::timeNow() << ']' << "Log initialized" << std::endl;
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

VKAPI_ATTR VkBool32 VKAPI_CALL Log::vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    (void)messageSeverity;
    (void)messageType;
    (void)userData;

    Log& log = Log::getInstance();
    log << "[Validation layer] " << callbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Log::setupDebugCallback(VkInstance vkInstance, VkDevice vkDevice)
{
#ifdef ENGINE_DEBUG
    pfn_vkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (pfn_vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        Log::getInstance() << "Renderer::setupDebugCallback(): Function vkGetInstanceProcAddr call for "
                              "vkCreateDebugUtilsMessengerEXT failed. Terminated."
                           << std::endl;
        std::terminate();
    }

    pfn_vkDestroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
    if (pfn_vkDestroyDebugUtilsMessengerEXT == nullptr)
    {
        Log::getInstance() << "Renderer::setupDebugCallback(): Function vkGetInstanceProcAddr call for "
                              "vkDestroyDebugUtilsMessengerEXT failed. Terminated."
                           << std::endl;
        std::terminate();
    }

    VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo = {};
    vkDebugUtilsMessengerCreateInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    vkDebugUtilsMessengerCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                      VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    vkDebugUtilsMessengerCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                  VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                  VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    vkDebugUtilsMessengerCreateInfo.pfnUserCallback = Log::vulkanDebugCallback;

    Log& log            = getInstance();
    VkResult resultCode = pfn_vkCreateDebugUtilsMessengerEXT(vkInstance, &vkDebugUtilsMessengerCreateInfo, nullptr, &log.m_vkDebugMessenger);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Log::setupDebugCallback(): Function vkCreateDebugUtilsMessengerEXT "
                              "call failed wih code "
                           << resultCode << std::endl;
    }
    log.m_vkDebugMessengerInstance = vkInstance;
#endif
}

void Log::deleteDebugCallback()
{
#ifdef ENGINE_DEBUG
    Log& log = getInstance();
    pfn_vkDestroyDebugUtilsMessengerEXT(log.m_vkDebugMessengerInstance, log.m_vkDebugMessenger, nullptr);
#endif
}
