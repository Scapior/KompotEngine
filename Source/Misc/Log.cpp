#include "Log.hpp"
#include "DateTimeStringFormatter.hpp"
#include <string>

#ifdef ENGINE_DEBUG
PFN_vkCreateDebugUtilsMessengerEXT  Log::pfn_vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT Log::pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;
#endif

Log::Log()
{
    using namespace KompotEngine;
    m_logFile.open("log.txt");
    const std::string time = DateTimeStringFormatter::now(DateTimeStringFormatter::logDateTimeFormat);
    *this << time << "Log initialized" << std::endl;
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

    Log &log = Log::getInstance();
    log << "[Validation layer] " << callbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Log::setupDebugCallback(VkInstance vkInstance, VkDevice vkDevice)
{
#ifdef ENGINE_DEBUG
    pfn_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (pfn_vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        Log::getInstance() << "Renderer::setupDebugCallback(): Function vkGetInstanceProcAddr call for vkCreateDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }

    pfn_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
    if (pfn_vkDestroyDebugUtilsMessengerEXT == nullptr)
    {
        Log::getInstance() << "Renderer::setupDebugCallback(): Function vkGetInstanceProcAddr call for vkDestroyDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }

    VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo = {};
    vkDebugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    vkDebugUtilsMessengerCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                      VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    vkDebugUtilsMessengerCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                  VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                  VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    vkDebugUtilsMessengerCreateInfo.pfnUserCallback = Log::vulkanDebugCallback;

    Log &log = getInstance();
    VkResult resultCode = pfn_vkCreateDebugUtilsMessengerEXT(vkInstance, &vkDebugUtilsMessengerCreateInfo, nullptr, &log.m_vkDebugMessenger);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Log::setupDebugCallback(): Function vkCreateDebugUtilsMessengerEXT call failed wih code " << resultCode << std::endl;
    }
    log.m_vkDebugMessengerInstance = vkInstance;
#endif
}

void Log::deleteDebugCallback()
{
#ifdef ENGINE_DEBUG
    Log &log = getInstance();
    pfn_vkDestroyDebugUtilsMessengerEXT(log.m_vkDebugMessengerInstance, log.m_vkDebugMessenger, nullptr);
#endif
}
