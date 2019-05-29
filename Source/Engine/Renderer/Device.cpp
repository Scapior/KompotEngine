#include "Device.hpp"

using namespace  KompotEngine::Renderer;

Device::Device(const std::string &applicationName)
    : m_applicationName(applicationName),
      m_vkInstance(nullptr),
      m_vkPhysicalDevice(nullptr),
      m_vkDevice(nullptr)
{
    createVkInstance();
    selectPhysicalDevice();
}

Device::~Device()
{
    if (m_vkDevice)
    {
        vkDestroyDevice(m_vkDevice, nullptr);
    }
    if (m_vkInstance)
    {
        vkDestroyInstance(m_vkInstance, nullptr);
    }
}

VkResult Device::create(VkSurfaceKHR vkSurface)
{
    m_vkSurface = vkSurface;
    return createDevice();
}

VkInstance Device::getInstance() const
{
    return m_vkInstance;
}

VkQueue Device::getGraphicsQueue() const
{
    return m_vkGraphicsQueue;
}

KompotEngine::Renderer::Device::operator VkDevice() const
{
    return m_vkDevice;
}

KompotEngine::Renderer::Device::operator VkPhysicalDevice() const
{
    return m_vkPhysicalDevice;
}

VkResult Device::createVkInstance()
{
    VkApplicationInfo vkApplicationInfo {};
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pApplicationName = m_applicationName.c_str();
    vkApplicationInfo.applicationVersion = VK_MAKE_VERSION(0_u8t, 0_u8t, 1_u8t);
    vkApplicationInfo.pEngineName = ENGINE_NAME;
    vkApplicationInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VESRION_MAJOR, ENGINE_VESRION_MINOR, ENGINE_VESRION_PATCH);
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_1;

    auto extensionsCount = 0_u32t;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    std::vector<const char*> extensions(extensionsCount);
    for (auto i = 0_u32t; i < extensionsCount; ++i)
    {
        extensions[i] = glfwExtensions[i];
    }
#ifdef ENGINE_DEBUG
    extensions.push_back("VK_EXT_debug_utils");
#endif

    VkInstanceCreateInfo vkInstanceInfo = {};
    vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
    vkInstanceInfo.ppEnabledLayerNames = validationLayers.data();
    vkInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    vkInstanceInfo.ppEnabledExtensionNames = extensions.data();

    const auto resultCode = vkCreateInstance(&vkInstanceInfo, nullptr, &m_vkInstance);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Device::createVkInstance(): Function vkCreateInstance call failed with a code " << resultCode << "." << std::endl;
    }
    return resultCode;
}

VkResult Device::selectPhysicalDevice()
{
    VkResult resultCode;
    auto physicalDevicesCount = 0_u32t;
    resultCode = vkEnumeratePhysicalDevices(m_vkInstance, &physicalDevicesCount, nullptr);
    if (resultCode != VK_SUCCESS)
    {
        return resultCode;
    }

    if (physicalDevicesCount == 0_u32t)
    {
        Log::getInstance() << "Device::selectPhysicalDevice(): No physical devices found." << std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDevicesCount);
    resultCode = vkEnumeratePhysicalDevices(m_vkInstance, &physicalDevicesCount, vkPhysicalDevices.data());
    if (resultCode != VK_SUCCESS)
    {
        return resultCode;
    }


    std::string lastDeviceName;
    auto lastPhysicalDeviceMemorySize = 0_u32t;
    for (auto &physicalDevice : vkPhysicalDevices)
    {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &vkPhysicalDeviceProperties);

        lastDeviceName = vkPhysicalDeviceProperties.deviceName;

        VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &vkPhysicalDeviceMemoryProperties);
        auto memoryHeaps = std::vector<VkMemoryHeap>(vkPhysicalDeviceMemoryProperties.memoryHeaps,
                                               vkPhysicalDeviceMemoryProperties.memoryHeaps + vkPhysicalDeviceMemoryProperties.memoryHeapCount);
        auto physicalDeviceMemorySize = 1_u32t;
        for (const auto& heap : memoryHeaps)
        {
            if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                physicalDeviceMemorySize = static_cast<uint32_t>(heap.size);
                break;
            }
        }

        if (vkPhysicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            if (physicalDeviceMemorySize > lastPhysicalDeviceMemorySize)
            {
                m_vkPhysicalDevice = physicalDevice;
            }
        }
        else if (vkPhysicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (m_vkPhysicalDevice == nullptr)
            {
                m_vkPhysicalDevice = physicalDevice;
            }
        }
    }

    if (m_vkPhysicalDevice == nullptr)
    {
        Log::getInstance() << "Device::selectPhysicalDevice(): No situable physical devices found" << std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    Log::getInstance() << "Device::selectPhysicalDevice(): Founded physical device \"" << lastDeviceName << "\"." << std::endl;
    return resultCode;
}

QueueFamilyIndices Device::findQueueFamilies()
{
    QueueFamilyIndices indices;
    auto queueFamiliesCount = 0_u32t;

    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> vkQueueFamiliesProperties(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamiliesCount, vkQueueFamiliesProperties.data());

    for ( auto i = 0_u32t; i < vkQueueFamiliesProperties.size(); ++i)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, m_vkSurface, &presentSupport);

        if (vkQueueFamiliesProperties[i].queueCount > 0_u32t && presentSupport)
        {
            indices.presentFamilyIndex = i;
        }

        if (vkQueueFamiliesProperties[i].queueCount > 0_u32t && vkQueueFamiliesProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicFamilyIndex = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    if (!indices.isComplete())
    {
        Log::getInstance() << "Renderer::findQueueFamilies(): Not all queue families found. Terminated." << std::endl;
        std::terminate();
    }

    return indices;
}

uint32_t Device::findMemoryType(uint32_t requiredTypes, VkMemoryPropertyFlags requiredProperties)
{
    VkPhysicalDeviceMemoryProperties vkPhysicalDevicememoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &vkPhysicalDevicememoryProperties);

    for (auto i = 0_u32t; i < vkPhysicalDevicememoryProperties.memoryTypeCount; ++i)
    {
        if ((requiredTypes & (1 << i)) && (vkPhysicalDevicememoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)
        {
            return i;
        }
    }

    Log::getInstance() << "Renderer::findMemoryType(): Failed to find suitable memory type. Terminated."<< std::endl;
    std::terminate();
}


VkResult Device::createDevice()
{
    if (!m_vkInstance || !m_vkPhysicalDevice)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const auto familiesIndecies = findQueueFamilies();
    std::set<uint32_t> indices = {
        familiesIndecies.graphicFamilyIndex.value(),
        familiesIndecies.presentFamilyIndex.value()
    };
    const auto queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> vkDeviceQueueCreateInfos;
    for (const auto familyIndex : indices)
    {
        VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo = {};
        vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        vkDeviceQueueCreateInfo.queueFamilyIndex = familyIndex;
        vkDeviceQueueCreateInfo.queueCount = 1_u32t;
        vkDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        vkDeviceQueueCreateInfos.push_back(vkDeviceQueueCreateInfo);
    }

    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures = {};
    vkPhysicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
    std::vector<const char*> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo vkDeviceCreateInfo = {};
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(indices.size());
    vkDeviceCreateInfo.pQueueCreateInfos = vkDeviceQueueCreateInfos.data();
    vkDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    vkDeviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    vkDeviceCreateInfo.ppEnabledExtensionNames = extensions.data();
    vkDeviceCreateInfo.pEnabledFeatures = &vkPhysicalDeviceFeatures;

    const auto resultCode = vkCreateDevice(m_vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &m_vkDevice);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Device::createDevice(): Function vkCreateDevice call failed with a code " << resultCode << "." << std::endl;
    }
    else
    {
        vkGetDeviceQueue(m_vkDevice, familiesIndecies.graphicFamilyIndex.value(), 0_u32t, &m_vkGraphicsQueue);
        vkGetDeviceQueue(m_vkDevice, familiesIndecies.presentFamilyIndex.value(), 0_u32t, &m_vkPresentQueue);
    }

    return resultCode;

}
