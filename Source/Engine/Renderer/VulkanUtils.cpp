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

    auto layersStrings = getLayers();
    std::vector<const char*> layers(layersStrings.size());
    std::transform(std::begin(layersStrings), std::end(layersStrings), std::begin(layers),
                   [&](const auto& layerString)
    {
        return layerString.c_str();
    });

    auto extensionsStrings = getExtensions();
    std::vector<const char*> extensions(extensionsStrings.size());
    std::transform(std::begin(extensionsStrings), std::end(extensionsStrings), std::begin(extensions),
                   [&](const auto& extensionString)
    {
        return extensionString.c_str();
    });

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = static_cast<unsigned int>(layers.size());
    instanceInfo.ppEnabledLayerNames = layers.data();
    instanceInfo.enabledExtensionCount = static_cast<unsigned int>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&instanceInfo, nullptr, &vkInstance) != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateInstance failed. Terminated." << std::endl;
        std::terminate();
    }
}

void KompotEngine::Renderer::loadFuntions(VkInstance vkInstance)
{
    Log &log = Log::getInstance();
    pfn_vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
    pfn_vkCreateDebugUtilsMessengerEXT  = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (pfn_vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        log << "vkGetInstanceProcAddr for vkCreateDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }
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

std::vector<std::string> KompotEngine::Renderer::getLayers()
{
    std::vector<std::string> layers;
    Log &log = Log::getInstance();
    std::vector<const char*> validationLayers {
        "VK_LAYER_LUNARG_standard_validation"
    };

    auto availablelayersCount = 0_u32t;
    vkEnumerateInstanceLayerProperties(&availablelayersCount, nullptr);
    std::vector<VkLayerProperties> availablelayers(availablelayersCount);
    vkEnumerateInstanceLayerProperties(&availablelayersCount, availablelayers.data());

    for (const auto& validationLayer : validationLayers)
    {
        const auto index = std::find_if(availablelayers.begin(), availablelayers.end(),
                                        [validationLayer](const auto& availablelayer) -> bool
        {
            return boost::iequals(availablelayer.layerName, validationLayer);
        });
        if (index == availablelayers.end())
        {
            log << "Required validation layer \"" <<  validationLayer << "\n not available." << std::endl;
            std::terminate();
        }
    }

    for (const auto& availablelayer : availablelayers)
    {
        layers.push_back(availablelayer.layerName);
    }    
    return layers;
}


std::vector<std::string> KompotEngine::Renderer::getExtensions()
{
    std::vector<std::string> extensions;
    Log &log = Log::getInstance();
    auto glfwRequiredExtensionsCount = 0_u32t;
    auto glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionsCount);
    auto availableVulkanExtensionsCount = 0_u32t;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableVulkanExtensionsCount, nullptr);
    std::vector<VkExtensionProperties> availableVulkanExtensions(availableVulkanExtensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableVulkanExtensionsCount, availableVulkanExtensions.data());
    for (auto i = 0_u32t; i < glfwRequiredExtensionsCount; ++i)
    {
        const auto &requiredExtensionName = glfwRequiredExtensions[i];
        const auto index = std::find_if(availableVulkanExtensions.begin(), availableVulkanExtensions.end(),
                                  [requiredExtensionName](const auto &extension) -> bool
        {
            return boost::iequals(extension.extensionName, requiredExtensionName);
        });
        if (index == availableVulkanExtensions.end())
        {
           log << "Required by GLFW extension \"" <<  requiredExtensionName << "\n not available." << std::endl;
           std::terminate();
        }
    }

    for (const auto &extensionProperty : availableVulkanExtensions)
    {
        extensions.push_back(extensionProperty.extensionName);
    }
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
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

    auto layersStrings = getLayers();
    std::vector<const char*> layers(layersStrings.size());
    std::transform(std::begin(layersStrings), std::end(layersStrings), std::begin(layers),
                   [&](const auto& layerString)
    {
        return layerString.c_str();
    });

//    auto extensionsStrings = getExtensions();
//    std::vector<const char*> extensions(extensionsStrings.size());
//    std::transform(std::begin(extensionsStrings), std::end(extensionsStrings), std::begin(extensions),
//                   [&](const auto& extensionString)
//    {
//        return extensionString.c_str();
//    });

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1_u32t;
    deviceInfo.pQueueCreateInfos = &graphicQueueInfo;
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    deviceInfo.ppEnabledLayerNames = layers.data();
    deviceInfo.enabledExtensionCount = 0_u32t;//static_cast<uint32_t>(extensions.size());
    deviceInfo.ppEnabledExtensionNames = nullptr; //extensions.data();
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
