#include "Log.hpp"

void Log::callbackForGlfw(int code, const char* text)
{
    Log &log = Log::getInstance();
    log << "GLFW error: code [" << code << "], text: " << text << std::endl;
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
