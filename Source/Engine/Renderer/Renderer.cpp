#include "Renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

using namespace KompotEngine::Renderer;

#ifdef ENGINE_DEBUG
PFN_vkCreateDebugUtilsMessengerEXT  Renderer::pfn_vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT Renderer::pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;
#endif

const std::vector<Vertex> vertices = {
    {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> verticesIndices = {0, 1, 2, 2, 1, 3};

Renderer::Renderer(GLFWwindow *window, const std::string &windowName)
    : m_glfwWindowHandler(window), m_windowsName(windowName), m_isResized(false)
{
    createVkInstance();
    setupDebugCallback();
    createSurface();
    selectPysicalDevice();
    createDevice();
    createSwapchain();
    createRenderPass();
    createFramebuffers();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void Renderer::cleanupSwapchain()
{
    for (auto framebuffer : m_vkFramebuffers)
    {
        vkDestroyFramebuffer(m_vkDevice, framebuffer, nullptr);
    }
    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, static_cast<uint32_t>(m_vkCommandBuffers.size()), m_vkCommandBuffers.data());
    vkDestroyPipeline(m_vkDevice, m_vkPipeline, nullptr);
    vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, nullptr);
    vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, nullptr);
    for (auto imageView : m_vkImageViews)
    {
        vkDestroyImageView(m_vkDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, nullptr);
}

Renderer::~Renderer()
{
    cleanupSwapchain();
    vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, nullptr);
    for (auto i = 0_u64t; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyBuffer(m_vkDevice, m_vkUniformMatricesBuffers[i], nullptr);
        vkFreeMemory(m_vkDevice, m_vkUniformMatricesBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);
    vkDestroySampler(m_vkDevice, m_vkTextureSampler, nullptr);
    vkDestroyImageView(m_vkDevice, m_vkTextureImageView, nullptr);
    vkDestroyImage(m_vkDevice, m_vkTextureImage, nullptr);
    vkFreeMemory(m_vkDevice, m_vkTextureImageMemory, nullptr);
    vkDestroyBuffer(m_vkDevice, m_vkIndexBuffer, nullptr);
    vkFreeMemory(m_vkDevice, m_vkIndexBufferMemory, nullptr);
    vkDestroyBuffer(m_vkDevice, m_vkVertexBuffer, nullptr);
    vkFreeMemory(m_vkDevice, m_vkVertexBufferMemory, nullptr);
    for (auto i = 0_u64t; i < MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        vkDestroySemaphore(m_vkDevice, m_vkImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_vkDevice, m_vkRenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_vkDevice, m_vkInFlightFramesFence[i], nullptr);
    }
    vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, nullptr);
    vkDestroyDevice(m_vkDevice, nullptr);
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
#ifdef ENGINE_DEBUG
    pfn_vkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, nullptr);
#endif
    vkDestroyInstance(m_vkInstance, nullptr);
}

void Renderer::run()
{
    Log &log = Log::getInstance();
    auto currentFrameIndex = 0_u64t;
    while (!glfwWindowShouldClose(m_glfwWindowHandler)) // todo: remove all this
    {
        currentFrameIndex = ++currentFrameIndex % MAX_FRAMES_IN_FLIGHT;
        auto &imageAvailableSemaphore = m_vkImageAvailableSemaphores[currentFrameIndex];
        auto &renderFinishedSemaphore = m_vkRenderFinishedSemaphores[currentFrameIndex];

        vkWaitForFences(m_vkDevice, 1_u32t, &m_vkInFlightFramesFence[currentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

        auto imageIndex = 0_u32t;
        auto resultCode = vkAcquireNextImageKHR(m_vkDevice, m_vkSwapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, nullptr, &imageIndex);

        if (resultCode == VK_ERROR_OUT_OF_DATE_KHR || m_isResized)
        {
            m_isResized = false;
            recreateSwapchain();
            continue;
        }

        updateUniformBuffer(imageIndex);

        VkPipelineStageFlags pipelineStagesFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1_u32t;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = pipelineStagesFlags;
        submitInfo.commandBufferCount = 1_u32t;
        submitInfo.pCommandBuffers = &m_vkCommandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1_u32t;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

        vkResetFences(m_vkDevice, 1_u32t, &m_vkInFlightFramesFence[currentFrameIndex]);
        resultCode = vkQueueSubmit(m_vkGraphicsQueue, 1_u32t, &submitInfo, m_vkInFlightFramesFence[currentFrameIndex]);
        if (resultCode != VK_SUCCESS)
        {
            log << "Renderer::run(): vkQueueSubmit failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1_u32t;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1_u32t;
        presentInfo.pSwapchains = &m_vkSwapchain;
        presentInfo.pImageIndices = &imageIndex;

        resultCode = vkQueuePresentKHR(m_vkGraphicsQueue, &presentInfo);
        if (resultCode == VK_ERROR_OUT_OF_DATE_KHR ||
            resultCode == VK_SUBOPTIMAL_KHR ||
            m_isResized)
        {
            m_isResized = false;
            recreateSwapchain();
        }
        else if(resultCode != VK_SUCCESS)
        {
            log << "Renderer::run(): vkQueuePresentKHR failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
        vkQueueWaitIdle(m_vkGraphicsQueue);
    }

    vkDeviceWaitIdle(m_vkDevice);
}

void Renderer::resize()
{
    m_isResized = true;
}

void Renderer::createVkInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = m_windowsName.c_str();
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
#ifdef ENGINE_DEBUG
    extensions.push_back("VK_EXT_debug_utils");
#endif

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    const auto resultCode = vkCreateInstance(&instanceInfo, nullptr, &m_vkInstance);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createVkInstance(): Function vkCreateInstance call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::setupDebugCallback()
{
#ifdef ENGINE_DEBUG
    Log &log = Log::getInstance();

    pfn_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (pfn_vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        log << "Renderer::setupDebugCallback(): Function vkGetInstanceProcAddr call for vkCreateDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }

    pfn_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
    if (pfn_vkDestroyDebugUtilsMessengerEXT == nullptr)
    {
        log << "Renderer::setupDebugCallback(): Function vkGetInstanceProcAddr call for vkDestroyDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                      VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCreateInfo.pfnUserCallback = Log::vulkanDebugCallback;

    VkResult resultCode = pfn_vkCreateDebugUtilsMessengerEXT(m_vkInstance, &messengerCreateInfo, nullptr, &m_vkDebugMessenger);
    if (resultCode != VK_SUCCESS)
    {
        log << "Renderer::setupDebugCallback(): Function vkCreateDebugUtilsMessengerEXT call failed wih code " << resultCode << std::endl;
    }
#endif
}

void Renderer::createSurface()
{
    const auto resultCode = glfwCreateWindowSurface(m_vkInstance, m_glfwWindowHandler, nullptr, &m_vkSurface);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createSurface(): glfwCreateWindowSurface failed wih code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::selectPysicalDevice()
{
    Log &log = Log::getInstance();

    auto physicalDevicesCount = 0_u32t;
    vkEnumeratePhysicalDevices(m_vkInstance, &physicalDevicesCount, nullptr);

    if (physicalDevicesCount == 0_u32t)
    {
        log << "Renderer::selectPysicalDevice(): No physical devices found. Terminated." << std::endl;
        std::terminate();
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(m_vkInstance, &physicalDevicesCount, physicalDevices.data());

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
                m_vkPhysicalDevice = physicalDevice;
            }
        }
        else if (physicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (m_vkPhysicalDevice == nullptr)
            {
                m_vkPhysicalDevice = physicalDevice;
            }
        }
    }

    if (m_vkPhysicalDevice == nullptr)
    {
        log << "Renderer::selectPysicalDevice(): No situable physical devices found" << std::endl;
        std::terminate();
    }

    log << "Renderer::selectPysicalDevice(): Founded physical device \"" << lastDeviceName << "\"." << std::endl;
}

QueueFamilyIndices Renderer::findQueueFamilies()
{
    QueueFamilyIndices indices;

    auto queueFamiliesCount = 0_u32t;
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamiliesCount, queueFamilies.data());

    for ( auto i = 0_u32t; i < queueFamilies.size(); ++i)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, m_vkSurface, &presentSupport);

        if (queueFamilies[i].queueCount > 0_u32t && presentSupport)
        {
            indices.presentFamilyIndex = i;
        }

        if (queueFamilies[i].queueCount > 0_u32t && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
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
        Log &log = Log::getInstance();
        log << "Renderer::findQueueFamilies(): Not all queue families found. Terminated." << std::endl;
        std::terminate();
    }

    return indices;
}

SwapchainSupportDetails Renderer::getSwapchainDetails()
{
    SwapchainSupportDetails swapchainDetails;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysicalDevice, m_vkSurface, &swapchainDetails.capabilities);

    auto formatsCount = 0_u32t;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &formatsCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &formatsCount, formats.data());
    swapchainDetails.format = chooseSurfaceFormat(formats);


    auto presentModesCount = 0_u32t;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &presentModesCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &presentModesCount, presentModes.data());
    swapchainDetails.presentMode = choosePresentMode(presentModes);

    return swapchainDetails;
}

void Renderer::createDevice()
{
    const auto familiesIndecies = findQueueFamilies();
    std::set<uint32_t> indices = {
        familiesIndecies.graphicFamilyIndex.value(),
        familiesIndecies.presentFamilyIndex.value()
    };
    const auto queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (const auto familyIndex : indices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //graphicQueueInfo.flags = ???
        queueCreateInfo.queueFamilyIndex = familyIndex;
        queueCreateInfo.queueCount = 1_u32t;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
    std::vector<const char*> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(indices.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    deviceInfo.ppEnabledLayerNames = validationLayers.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    deviceInfo.ppEnabledExtensionNames = extensions.data();
    deviceInfo.pEnabledFeatures = &physicalDeviceFeatures;

    const auto resultCode = vkCreateDevice(m_vkPhysicalDevice, &deviceInfo, nullptr, &m_vkDevice);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createDevice(): Function vkCreateDevice call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    vkGetDeviceQueue(m_vkDevice, familiesIndecies.graphicFamilyIndex.value(), 0_u32t, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkDevice, familiesIndecies.presentFamilyIndex.value(), 0_u32t, &m_vkPresentQueue);
}

VkSurfaceFormatKHR Renderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats)
{
    if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for(const auto &surfaceFormat: surfaceFormats)
    {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surfaceFormat;
        }
    }

    return surfaceFormats[0];
}

VkExtent2D Renderer::chooseExtent(const VkSurfaceCapabilitiesKHR &vkSurfaceCapabilities)
{
    glfwGetFramebufferSize(m_glfwWindowHandler, reinterpret_cast<int*>(&m_width), reinterpret_cast<int*>(&m_height));
    VkExtent2D extent = {m_width, m_height};
    extent.width = std::clamp(extent.width, vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, vkSurfaceCapabilities.minImageExtent.height, vkSurfaceCapabilities.maxImageExtent.height);
    return extent;
}

VkPresentModeKHR Renderer::choosePresentMode(const std::vector<VkPresentModeKHR> &presentModes)
{
    for (const auto &presentMode : presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void Renderer::createSwapchain()
{
    Log &log = Log::getInstance();
    const auto swapchainDetails = getSwapchainDetails();
    const auto queuefamilies = findQueueFamilies();
    uint32_t queuefamiliesIndicies[] = {
        queuefamilies.graphicFamilyIndex.value(),
        queuefamilies.presentFamilyIndex.value()
    };

    auto imageExtent = chooseExtent(swapchainDetails.capabilities);
    m_vkImageFormat = swapchainDetails.format.format;

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = m_vkSurface;
    swapchainInfo.minImageCount = swapchainDetails.capabilities.minImageCount + 1_u32t;
    swapchainInfo.imageFormat = swapchainDetails.format.format;
    swapchainInfo.imageColorSpace = swapchainDetails.format.colorSpace;
    swapchainInfo.imageExtent = imageExtent;
    swapchainInfo.imageArrayLayers = 1_u32t;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (queuefamilies.graphicFamilyIndex != queuefamilies.presentFamilyIndex)
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2_u32t;
        swapchainInfo.pQueueFamilyIndices = queuefamiliesIndicies;
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainInfo.preTransform = swapchainDetails.capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = swapchainDetails.presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = nullptr;

    const auto resultCode = vkCreateSwapchainKHR(m_vkDevice, &swapchainInfo, nullptr, &m_vkSwapchain);
    if (resultCode != VK_SUCCESS)
    {
        log << "Renderer::createSwapchain(): Function vkCreateSwapchainKHR call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
    auto swapchainImagesCount = 0_u32t;
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &swapchainImagesCount, nullptr);
    m_vkImages.resize(swapchainImagesCount);
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &swapchainImagesCount, m_vkImages.data());

    m_vkImageViews.resize(swapchainImagesCount);
    for (auto i = 0_u32t; i < swapchainImagesCount; ++i)
    {
        createImageView(m_vkImages[i], m_vkImageFormat, m_vkImageViews[i]);
    }
    m_vkViewport.x = 0.0f;
    m_vkViewport.y = 0.0f;
    m_vkViewport.height = static_cast<float>(m_height);
    m_vkViewport.width = static_cast<float>(m_width);
    m_vkViewport.minDepth = 0.0f;
    m_vkViewport.maxDepth = 1.0f;

    m_vkRect.offset = {0_32t, 0_32t};
    m_vkRect.extent = {m_width, m_height};
}

void Renderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_vkImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0_u32t;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1_u32t;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = 0_u32t;
    subpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1_u32t;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1_u32t;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.pDependencies = &subpassDependency;

    const auto resultCode = vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_vkRenderPass);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createRenderPass(): Function vkCreateRenderPass call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createFramebuffers()
{
    Log &log = Log::getInstance();
    m_vkFramebuffers.resize(static_cast<uint32_t>(m_vkImageViews.size()));
    for (auto i = 0_u64t; i < static_cast<uint64_t>(m_vkImageViews.size()); ++i)
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_vkRenderPass;
        framebufferInfo.attachmentCount = 1_u32t;//static_cast<uint32_t>(swapchain.imagesViews.size());
        framebufferInfo.pAttachments = &m_vkImageViews[i];
        framebufferInfo.width = m_width;
        framebufferInfo.height = m_height;
        framebufferInfo.layers = 1_u32t;

        const auto resultCode = vkCreateFramebuffer(m_vkDevice, &framebufferInfo, nullptr, &m_vkFramebuffers[i]);
        if (resultCode != VK_SUCCESS)
        {
            log << "Renderer::createFramebuffers(): Function vkCreateFramebuffer call failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
    }
}

void Renderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
    descriptorSetLayoutBinding.binding = 0_u32t;
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1_u32t;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1_u32t;
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

    const auto resultCode = vkCreateDescriptorSetLayout(m_vkDevice, &descriptorSetLayoutCreateInfo, nullptr, &m_vkDescriptorSetLayout);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createDescriptorSetLayout(): Function vkCreateDescriptorSetLayout call failed with code " << resultCode << ". Terminated." << std::endl;
    }
};

void Renderer::createUniformBuffer()
{
    VkDeviceSize uniformSize = sizeof(UnifromBufferObject);

    m_vkUniformMatricesBuffers.resize(m_vkImages.size());
    m_vkUniformMatricesBuffersMemory.resize(m_vkImages.size());

    for (auto i = 0_u64t; i < m_vkImages.size(); ++i)
    {
        createBuffer(uniformSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_vkUniformMatricesBuffers[i],
                     m_vkUniformMatricesBuffersMemory[i]);
    }
};

void Renderer::createDescriptorPool()
{
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize.descriptorCount = static_cast<uint32_t>(m_vkImages.size());

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = 1_u32t;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(m_vkImages.size());

    const auto resultCode = vkCreateDescriptorPool(m_vkDevice, &descriptorPoolCreateInfo, nullptr, &m_vkDescriptorPool);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createDescriptorPool(): Function vkCreateDescriptorPool call failed with code " << resultCode << ". Terminated." << std::endl;
    }
}

void Renderer::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_vkImages.size(), m_vkDescriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_vkDescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(m_vkImages.size());
    descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

    m_vkDescriptorSets.resize(m_vkImages.size());

    const auto resultCode = vkAllocateDescriptorSets(m_vkDevice, &descriptorSetAllocateInfo, m_vkDescriptorSets.data());
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createDescriptorSets(): Function vkAllocateDescriptorSets call failed with code " << resultCode << ". Terminated." << std::endl;
    }

    for (auto i = 0_u64t; i < m_vkImages.size(); ++i)
    {
        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer = m_vkUniformMatricesBuffers[i];
        descriptorBufferInfo.offset = 0_64t;
        descriptorBufferInfo.range = sizeof(UnifromBufferObject);

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = m_vkDescriptorSets[i];
        writeDescriptorSet.dstBinding = 0_u32t;
        writeDescriptorSet.dstArrayElement = 0_u32t;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1_u32t;
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

        vkUpdateDescriptorSets(m_vkDevice, 1_u32t, &writeDescriptorSet, 0_u32t, nullptr);
    }
}

void Renderer::createGraphicsPipeline()
{
    Log &log = Log::getInstance();
    Shader vertexShader("triangle.vert.spv", m_vkDevice);
    Shader fragmentShader("triangle.frag.spv", m_vkDevice);
    if (!vertexShader.load())
    {
        log << "Renderer::createGraphicsPipeline(): Vertex shader loading error. Terminated." << std::endl;
    }
    if (!fragmentShader.load())
    {
        log << "Renderer::createGraphicsPipeline(): Fragment shader loading error. Terminated." << std::endl;
    }

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShader.get();
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShader.get();
    fragmentShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragmentShaderStageInfo};

    auto vertexBindingDescription = Vertex::getBindingDescription();
    auto vertexAttributesDescriptions = Vertex::getAttributeDescritptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1_u32t;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributesDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributesDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

//    VkPipelineTessellationStateCreateInfo tessellationInfo = {};
//    tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
//    tessellationInfo

    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1_u32t;
    viewportInfo.pViewports = &m_vkViewport;
    viewportInfo.scissorCount = 1_u32t;
    viewportInfo.pScissors = &m_vkRect;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

//    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
//    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.attachmentCount = 1_u32t;
    colorBlendInfo.pAttachments = &colorBlendAttachment;

    //VkPipelineDynamicStateCreateInfo dynamicInfo = {};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1_u32t;
    pipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;

    auto resultCode = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout);
    if (resultCode != VK_SUCCESS)
    {
        log << "Renderer::createGraphicsPipeline(): Function vkCreatePipelineLayout call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2_u32t;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pViewportState = &viewportInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    pipelineCreateInfo.layout = m_vkPipelineLayout;
    pipelineCreateInfo.renderPass = m_vkRenderPass;
    pipelineCreateInfo.subpass = 0_u32t;

    resultCode = vkCreateGraphicsPipelines(m_vkDevice, nullptr, 1_u32t, &pipelineCreateInfo, nullptr, &m_vkPipeline);
    if (resultCode != VK_SUCCESS)
    {
        log << "Renderer::createGraphicsPipeline(): Function vkCreateGraphicsPipelines call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createCommandPool()
{
    VkCommandPoolCreateInfo commandPoolInfo = {};
    const auto queueFamilies = findQueueFamilies();
    commandPoolInfo.sType  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFamilies.graphicFamilyIndex.value();

    const auto resultCode = vkCreateCommandPool(m_vkDevice, &commandPoolInfo, nullptr, &m_vkCommandPool);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createCommandPool(): Function vkCreateCommandPool failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createCommandBuffers()
{
    Log &log = Log::getInstance();
    m_vkCommandBuffers.resize(m_vkFramebuffers.size());

    VkCommandBufferAllocateInfo commandBuffersAllocateInfo = {};
    commandBuffersAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBuffersAllocateInfo.commandPool = m_vkCommandPool;
    commandBuffersAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBuffersAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_vkCommandBuffers.size());

    auto resultCode = vkAllocateCommandBuffers(m_vkDevice, &commandBuffersAllocateInfo, m_vkCommandBuffers.data());
    if (resultCode != VK_SUCCESS)
    {
        log << "Renderer::createCommandBuffers(): Function vkAllocateCommandBuffers call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    for (auto i = 0_u64t; i < m_vkCommandBuffers.size(); ++i)
    {
        Log &log = Log::getInstance();

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        resultCode = vkBeginCommandBuffer(m_vkCommandBuffers[i], &commandBufferBeginInfo);
         if (resultCode != VK_SUCCESS)
        {
            log << "Renderer::createCommandBuffers(): Function vkBeginCommandBuffer call failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_vkRenderPass;
        renderPassBeginInfo.framebuffer = m_vkFramebuffers[i];
        renderPassBeginInfo.renderArea.offset = {0_32t, 0_32t};
        renderPassBeginInfo.renderArea.extent = { m_width, m_height};
        VkClearValue clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount = 1_u32t;
        renderPassBeginInfo.pClearValues = &clearValue;

        VkDeviceSize offsets[] = {0_u32t};
        vkCmdBeginRenderPass(m_vkCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
            vkCmdBindVertexBuffers(m_vkCommandBuffers[i], 0_u32t, 1_u32t, &m_vkVertexBuffer, offsets);
            vkCmdBindIndexBuffer(m_vkCommandBuffers[i], m_vkIndexBuffer, 0_u32t, VK_INDEX_TYPE_UINT16);
            vkCmdBindDescriptorSets(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0_u32t, 1_u32t, &m_vkDescriptorSets[i],  0_u32t, nullptr);
            vkCmdDrawIndexed(m_vkCommandBuffers[i], static_cast<uint32_t>(verticesIndices.size()), 1_u32t, 0_u32t, 0_u32t, 0_u32t);
        vkCmdEndRenderPass(m_vkCommandBuffers[i]);

        resultCode = vkEndCommandBuffer(m_vkCommandBuffers[i]);
        if (resultCode != VK_SUCCESS)
        {
            log << "Renderer::createCommandBuffers(): Function vkEndCommandBuffer call failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
    }
}
void Renderer::createSyncObjects()
{
    m_vkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkInFlightFramesFence.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = 0_u64t; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (VK_SUCCESS != vkCreateSemaphore(m_vkDevice, &semaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphores[i]) ||
            VK_SUCCESS != vkCreateSemaphore(m_vkDevice, &semaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphores[i]) ||
            VK_SUCCESS != vkCreateFence(m_vkDevice, &fenceCreateInfo, nullptr, &m_vkInFlightFramesFence[i]))
        {
            Log &log = Log::getInstance();
            log << "Failed to create synchronization objects. Terminated." << std::endl;
            std::terminate();
        }
    }
}

void Renderer::recreateSwapchain()
{
    vkDeviceWaitIdle(m_vkDevice);
    cleanupSwapchain();
    createSwapchain();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandBuffers();
}

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperies, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto resultCode = vkCreateBuffer(m_vkDevice, &bufferCreateInfo, nullptr, &buffer);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createBuffer(): Function vkCreateBuffer call failed with code" << resultCode << ". Terminated."<< std::endl;
        std::terminate();
    }

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(m_vkDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo momoryAllocateInfo = {};
    momoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    momoryAllocateInfo.allocationSize = memoryRequirements.size;
    momoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryProperies);

    resultCode = vkAllocateMemory(m_vkDevice, &momoryAllocateInfo, nullptr, &bufferMemory);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createBuffer(): Function vkAllocateMemory call failed with code" << resultCode << ". Terminated."<< std::endl;
        std::terminate();
    }

    vkBindBufferMemory(m_vkDevice, buffer, bufferMemory, 0_u32t);
}

void Renderer::copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size)
{
    auto commandBuffer = beginSingleTimeCommands();
    VkBufferCopy bufferCopyRegion = {};
    bufferCopyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1_u32t, &bufferCopyRegion);
    endSingleTimeCommands(commandBuffer);
}

void Renderer::createVertexBuffer()
{
    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void* bufferMemory;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0_u64t, size, 0_u32t, &bufferMemory);
    memcpy(bufferMemory, vertices.data(), size);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    createBuffer(size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_vkVertexBuffer,
                 m_vkVertexBufferMemory);

    copyBuffer(stagingBuffer, m_vkVertexBuffer, size);


    vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);
}

void Renderer::createIndexBuffer()
{
    VkDeviceSize size = sizeof(verticesIndices[0]) * verticesIndices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *bufferMemory;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0_u64t, size, 0_u32t, &bufferMemory);
    memcpy(bufferMemory, verticesIndices.data(), size);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    createBuffer(size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_vkIndexBuffer,
                 m_vkIndexBufferMemory);
    copyBuffer(stagingBuffer, m_vkIndexBuffer, size);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);
}

uint32_t Renderer::findMemoryType(uint32_t requiredTypes, VkMemoryPropertyFlags requiredProperties)
{
    VkPhysicalDeviceMemoryProperties physicalDevicememoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &physicalDevicememoryProperties);

    for (auto i = 0_u32t; i < physicalDevicememoryProperties.memoryTypeCount; ++i)
    {
        if ((requiredTypes & (1 << i)) && (physicalDevicememoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)
        {
            return i;
        }
    }

    Log &log = Log::getInstance();
    log << "Renderer::findMemoryType(): Failed to find suitable memory type. Terminated."<< std::endl;
    std::terminate();
}

void Renderer::updateUniformBuffer(uint32_t swapchainImageIndex)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UnifromBufferObject mvpMatrix = {};
    mvpMatrix.model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvpMatrix.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvpMatrix.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.01f, 10.0f);
    mvpMatrix.projection[1][1] *= -1; // flip Y (OGL coordinate system to VK)

    void *uniformData;
    vkMapMemory(m_vkDevice, m_vkUniformMatricesBuffersMemory[swapchainImageIndex], 0_u64t, sizeof(UnifromBufferObject), 0_u64t, &uniformData);
    memcpy(uniformData, &mvpMatrix, sizeof(UnifromBufferObject));
    vkUnmapMemory(m_vkDevice, m_vkUniformMatricesBuffersMemory[swapchainImageIndex]);
}


void Renderer::createImage(uint32_t width,
                 uint32_t height,
                 VkFormat format,
                 VkImageTiling tiling,
                 VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties,
                 VkImage &image,
                 VkDeviceMemory &imageMemory)
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1_u32t;
    imageCreateInfo.mipLevels = 1_u32t;
    imageCreateInfo.arrayLayers = 1_u32t;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    auto resultCode = vkCreateImage(m_vkDevice, &imageCreateInfo, nullptr, &image);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createImage(): Function vkCreateImage call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(m_vkDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    resultCode = vkAllocateMemory(m_vkDevice, &memoryAllocateInfo, nullptr, &imageMemory);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createImage(): Function vkAllocateMemory call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    vkBindImageMemory(m_vkDevice, image, imageMemory, 0_u64t);
}

void Renderer::createTextureImage()
{
    int textureWidth, textureHeight, textureChannels;
    stbi_uc *pixelsData = stbi_load("texture.jpg", &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = static_cast<uint64_t>(textureWidth) * static_cast<uint64_t>(textureHeight) * 4_u64t;

    if (!pixelsData)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createTextureImage(): Failed to load texture image." << std::endl;
        std::terminate();
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);
    void *stagingBufferData;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0_u64t, imageSize, 0_u64t, &stagingBufferData);
    memcpy(stagingBufferData, pixelsData, imageSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);
    stbi_image_free(pixelsData);


    createImage(static_cast<uint32_t>(textureWidth),
                static_cast<uint32_t>(textureHeight),
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vkTextureImage,
                m_vkTextureImageMemory);

    transitionImageLayout(m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, m_vkTextureImage, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight));
    transitionImageLayout(m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = m_vkCommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1_u32t;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_vkDevice, &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(m_vkGraphicsQueue);

    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &commandBuffer);
}

void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    auto commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0_u32t;
    region.bufferRowLength = 0_u32t;
    region.bufferImageHeight = 0_u32t;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0_u32t;
    region.imageSubresource.baseArrayLayer = 0_u32t;
    region.imageSubresource.layerCount = 1_u32t;
    region.imageOffset = {0_32t, 0_32t, 0_32t};
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1_u32t;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1_u32t, &region);

    endSingleTimeCommands(commandBuffer);
}

void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0_u32t;
    imageMemoryBarrier.subresourceRange.levelCount = 1_u32t;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0_u32t;
    imageMemoryBarrier.subresourceRange.layerCount = 1_u32t;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = 0_u32t;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        Log &log = Log::getInstance();
        log << "Renderer::transitionImageLayout(): Failed to load texture image." << std::endl;
        std::terminate();
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0_u32t, 0_u32t, nullptr, 0_u32t, nullptr, 1_u32t, &imageMemoryBarrier);
    endSingleTimeCommands(commandBuffer);
}

void Renderer::createImageView(VkImage image, VkFormat format, VkImageView &imageView)
{
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0_u32t;
    imageViewCreateInfo.subresourceRange.levelCount = 1_u32t;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0_u32t;
    imageViewCreateInfo.subresourceRange.layerCount = 1_u32t;


    const auto resultCode = vkCreateImageView(m_vkDevice, &imageViewCreateInfo, nullptr, &imageView);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createImageView(): Failed to load texture image view." << std::endl;
        std::terminate();
    }
}

void Renderer::createTextureImageView()
{
    createImageView(m_vkTextureImage, VK_FORMAT_R8G8B8A8_UNORM, m_vkTextureImageView);
}

void Renderer::createTextureSampler()
{
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16_u32t;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    const auto resultCode = vkCreateSampler(m_vkDevice, &samplerCreateInfo, nullptr, &m_vkTextureSampler);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "Renderer::createTextureSampler(): Function vkCreateImageView call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}
