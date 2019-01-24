#include "VulkanUtils.hpp"

using namespace  KompotEngine::Renderer;

void KompotEngine::Renderer::createVkInstance(VkInstance &vkInstance, const std::string &windowName)
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = windowName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0_u8t, 0_u8t, 1_u8t);
    applicationInfo.pEngineName = ENGINE_NAME.c_str();
    applicationInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VESRION_MAJOR, ENGINE_VESRION_MINOR, ENGINE_VESRION_PATCH);
    applicationInfo.apiVersion = VK_API_VERSION_1_1;

    auto extensionsCount = 0_u32t;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    std::vector<const char*> extensions(extensionsCount);
    for (auto i = 0_u32t; i < extensionsCount; ++i)
    {
        extensions[i] = glfwExtensions[i];
    }
    extensions.push_back("VK_EXT_debug_utils");

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    const auto resultCode = vkCreateInstance(&instanceInfo, nullptr, &vkInstance);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateInstance failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void KompotEngine::Renderer::loadFuntions(VkInstance vkInstance)
{
    Log &log = Log::getInstance();

    pfn_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (pfn_vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        log << "vkGetInstanceProcAddr for vkCreateDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }

    pfn_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
    if (pfn_vkDestroyDebugUtilsMessengerEXT == nullptr)
    {
        log << "vkGetInstanceProcAddr for vkDestroyDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }
}

void KompotEngine::Renderer::setupDebugCallback(VkInstance vkInstance, VkDebugUtilsMessengerEXT &vkDebugMessenger)
{
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                      VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCreateInfo.pfnUserCallback = Log::vulkanDebugCallback;

    VkResult debugMessengerCreatingCode = pfn_vkCreateDebugUtilsMessengerEXT(vkInstance, &messengerCreateInfo, nullptr, &vkDebugMessenger);
    if (debugMessengerCreatingCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateDebugUtilsMessengerEXT failed wih code " << debugMessengerCreatingCode << std::endl;
    }
}

void KompotEngine::Renderer::selectPhysicalDevice(VkInstance vkInstance, VkPhysicalDevice &vkPhysicalDevice)
{
    Log &log = Log::getInstance();

    auto physicalDevicesCount = 0_u32t;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, nullptr);

    if (physicalDevicesCount == 0_u32t)
    {
        log << "No physical devices found" << std::endl;
        std::terminate();
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, physicalDevices.data());

    std::string lastDeviceName;
    auto lastPhysicalDeviceMemorySize = 0_u32t;
    for (auto &physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        lastDeviceName = physicalDeviceProperties.deviceName;

        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
        auto memoryHeaps = std::vector<VkMemoryHeap>(physicalDeviceMemoryProperties.memoryHeaps,
                                               physicalDeviceMemoryProperties.memoryHeaps + physicalDeviceMemoryProperties.memoryHeapCount);
        auto physicalDeviceMemorySize = 1_u32t;
        for (const auto& heap : memoryHeaps)
        {
            if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                physicalDeviceMemorySize = static_cast<uint32_t>(heap.size);
                break;
            }
        }

        if (physicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            if (physicalDeviceMemorySize > lastPhysicalDeviceMemorySize)
            {
                vkPhysicalDevice = physicalDevice;
            }
        }
        else if (physicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (vkPhysicalDevice == nullptr)
            {
                vkPhysicalDevice = physicalDevice;
            }
        }
    }

    if (vkPhysicalDevice == nullptr)
    {
        log << "No situable devices found" << std::endl;
        std::terminate();
    }

    log << "Founded physical device \"" << lastDeviceName << "\"." << std::endl;
}

QueueFamilyIndices KompotEngine::Renderer::findQueueFamilies(VkPhysicalDevice vkPhysicalDevice)
{
    QueueFamilyIndices indices;

    auto queueFamiliesCount = 0_u32t;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamiliesCount, queueFamilies.data());

    for ( auto i = 0_u32t; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueCount > 0_u32t && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicIndex = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    if (!indices.isComplete())
    {
        Log &log = Log::getInstance();
        log << "Not all queue families found. Terminated." << std::endl;
        std::terminate();
    }

    return indices;
}

void KompotEngine::Renderer::createLogicalDeviceAndQueue(
        VkPhysicalDevice vkPhysicalDevice,
        VkDevice &vkDevice,
        VkQueue &graphicQueue)
{
    //VkQueueFamilyProperties
    const auto familiesIndexes = findQueueFamilies(vkPhysicalDevice);
    const auto queuePriority = 1.0f;

    VkDeviceQueueCreateInfo graphicQueueInfo = {};
    graphicQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    //graphicQueueInfo.flags = ???
    graphicQueueInfo.queueFamilyIndex = familiesIndexes.graphicIndex.value();
    graphicQueueInfo.queueCount = 1_u32t;
    graphicQueueInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};

    std::vector<const char*> extensions = {
        "VK_KHR_swapchain"
    };

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1_u32t;
    deviceInfo.pQueueCreateInfos = &graphicQueueInfo;
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    deviceInfo.ppEnabledLayerNames = validationLayers.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    deviceInfo.ppEnabledExtensionNames = extensions.data();
    deviceInfo.pEnabledFeatures = &physicalDeviceFeatures;

    const auto resultCode = vkCreateDevice(vkPhysicalDevice, &deviceInfo, nullptr, &vkDevice);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateDevice failed. Terminated." << std::endl;
        std::terminate();
    }

    vkGetDeviceQueue(vkDevice, familiesIndexes.graphicIndex.value(), 0_u32t, &graphicQueue);
}
