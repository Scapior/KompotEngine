#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

#ifdef ENGINE_DEBUG
PFN_vkCreateDebugUtilsMessengerEXT  Renderer::pfn_vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT Renderer::pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;
#endif

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
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();

    m_resourcesMaker = new ResourcesMaker(m_vkPhysicalDevice, m_vkDevice, m_vkCommandPool, m_vkGraphicsQueue);

    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    IO::ModelsLoader modelsLoader;
    modelsLoader.loadFile("cube.kem");
    m_model = modelsLoader.generateModel();

    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createDepthResources();
    createFramebuffers();
    createCommandBuffers();
    createSyncObjects();
}

void Renderer::cleanupSwapchain()
{
    delete m_resourcesMaker;
    vkDestroyImageView(m_vkDevice, m_vkDepthImageView, nullptr);
    vkDestroyImage(m_vkDevice, m_vkDepthImage, nullptr);
    vkFreeMemory(m_vkDevice, m_vkDepthImageMemory, nullptr);
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
        m_vkUniformMatricesBuffers[i].reset();
    }
    vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);
    vkDestroySampler(m_vkDevice, m_vkTextureSampler, nullptr);
    vkDestroyImageView(m_vkDevice, m_vkTextureImageView, nullptr);
    vkDestroyImage(m_vkDevice, m_vkTextureImage, nullptr);
    vkFreeMemory(m_vkDevice, m_vkTextureImageMemory, nullptr);
    m_vkVertexBuffer.reset();
    m_vkIndexBuffer.reset();
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
            Log::getInstance() << "Renderer::run(): vkQueueSubmit failed with code " << resultCode << ". Terminated." << std::endl;
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
            Log::getInstance() << "Renderer::run(): vkQueuePresentKHR failed with code " << resultCode << ". Terminated." << std::endl;
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
    VkApplicationInfo vkApplicationInfo = {};
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pApplicationName = m_windowsName.c_str();
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
        Log::getInstance() << "Renderer::createVkInstance(): Function vkCreateInstance call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::setupDebugCallback()
{
#ifdef ENGINE_DEBUG
    pfn_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (pfn_vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        Log::getInstance() << "Renderer::setupDebugCallback(): Function vkGetInstanceProcAddr call for vkCreateDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }

    pfn_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
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

    VkResult resultCode = pfn_vkCreateDebugUtilsMessengerEXT(m_vkInstance, &vkDebugUtilsMessengerCreateInfo, nullptr, &m_vkDebugMessenger);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::setupDebugCallback(): Function vkCreateDebugUtilsMessengerEXT call failed wih code " << resultCode << std::endl;
    }
#endif
}

void Renderer::createSurface()
{
    const auto resultCode = glfwCreateWindowSurface(m_vkInstance, m_glfwWindowHandler, nullptr, &m_vkSurface);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createSurface(): glfwCreateWindowSurface failed wih a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::selectPysicalDevice()
{
    auto physicalDevicesCount = 0_u32t;
    vkEnumeratePhysicalDevices(m_vkInstance, &physicalDevicesCount, nullptr);

    if (physicalDevicesCount == 0_u32t)
    {
        Log::getInstance() << "Renderer::selectPysicalDevice(): No physical devices found. Terminated." << std::endl;
        std::terminate();
    }

    std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(m_vkInstance, &physicalDevicesCount, vkPhysicalDevices.data());

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
        Log::getInstance() << "Renderer::selectPysicalDevice(): No situable physical devices found" << std::endl;
        std::terminate();
    }

    Log::getInstance() << "Renderer::selectPysicalDevice(): Founded physical device \"" << lastDeviceName << "\"." << std::endl;
}

QueueFamilyIndices Renderer::findQueueFamilies()
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

SwapchainSupportDetails Renderer::getSwapchainDetails()
{
    SwapchainSupportDetails swapchainDetails;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysicalDevice, m_vkSurface, &swapchainDetails.capabilities);

    auto formatsCount = 0_u32t;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &formatsCount, nullptr);
    std::vector<VkSurfaceFormatKHR> vkSurfaceFormats(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &formatsCount, vkSurfaceFormats.data());
    swapchainDetails.format = chooseSurfaceFormat(vkSurfaceFormats);

    auto presentModesCount = 0_u32t;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &presentModesCount, nullptr);
    std::vector<VkPresentModeKHR> vkPresentModes(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &presentModesCount, vkPresentModes.data());
    swapchainDetails.presentMode = choosePresentMode(vkPresentModes);

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
        Log::getInstance() << "Renderer::createDevice(): Function vkCreateDevice call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    vkGetDeviceQueue(m_vkDevice, familiesIndecies.graphicFamilyIndex.value(), 0_u32t, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkDevice, familiesIndecies.presentFamilyIndex.value(), 0_u32t, &m_vkPresentQueue);
}

VkSurfaceFormatKHR Renderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &vkSurfaceFormats)
{
    if (vkSurfaceFormats.size() == 1 && vkSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for(const auto &surfaceFormat: vkSurfaceFormats)
    {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surfaceFormat;
        }
    }

    return vkSurfaceFormats[0];
}

VkExtent2D Renderer::chooseExtent(const VkSurfaceCapabilitiesKHR &vkSurfaceCapabilities)
{
    glfwGetFramebufferSize(m_glfwWindowHandler, reinterpret_cast<int*>(&m_width), reinterpret_cast<int*>(&m_height));
    VkExtent2D vkExtent2D = {m_width, m_height};
    vkExtent2D.width = std::clamp(vkExtent2D.width, vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.maxImageExtent.width);
    vkExtent2D.height = std::clamp(vkExtent2D.height, vkSurfaceCapabilities.minImageExtent.height, vkSurfaceCapabilities.maxImageExtent.height);
    return vkExtent2D;
}

VkPresentModeKHR Renderer::choosePresentMode(const std::vector<VkPresentModeKHR> &vkPresentModes)
{
    for (const auto &presentMode : vkPresentModes)
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
    const auto swapchainDetails = getSwapchainDetails();
    const auto queuefamilies = findQueueFamilies();
    uint32_t queuefamiliesIndicies[] = {
        queuefamilies.graphicFamilyIndex.value(),
        queuefamilies.presentFamilyIndex.value()
    };

    auto imageExtent = chooseExtent(swapchainDetails.capabilities);
    m_vkImageFormat = swapchainDetails.format.format;

    VkSwapchainCreateInfoKHR vkSwapchainCreateInfo = {};
    vkSwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfo.surface = m_vkSurface;
    vkSwapchainCreateInfo.minImageCount = swapchainDetails.capabilities.minImageCount + 1_u32t;
    vkSwapchainCreateInfo.imageFormat = swapchainDetails.format.format;
    vkSwapchainCreateInfo.imageColorSpace = swapchainDetails.format.colorSpace;
    vkSwapchainCreateInfo.imageExtent = imageExtent;
    vkSwapchainCreateInfo.imageArrayLayers = 1_u32t;
    vkSwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (queuefamilies.graphicFamilyIndex != queuefamilies.presentFamilyIndex)
    {
        vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        vkSwapchainCreateInfo.queueFamilyIndexCount = 2_u32t;
        vkSwapchainCreateInfo.pQueueFamilyIndices = queuefamiliesIndicies;
    }
    else
    {
        vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    vkSwapchainCreateInfo.preTransform = swapchainDetails.capabilities.currentTransform;
    vkSwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfo.presentMode = swapchainDetails.presentMode;
    vkSwapchainCreateInfo.clipped = VK_TRUE;
    vkSwapchainCreateInfo.oldSwapchain = nullptr;

    const auto resultCode = vkCreateSwapchainKHR(m_vkDevice, &vkSwapchainCreateInfo, nullptr, &m_vkSwapchain);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createSwapchain(): Function vkCreateSwapchainKHR call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
    auto swapchainImagesCount = 0_u32t;
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &swapchainImagesCount, nullptr);
    m_vkImages.resize(swapchainImagesCount);
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &swapchainImagesCount, m_vkImages.data());

    m_vkImageViews.resize(swapchainImagesCount);
    for (auto i = 0_u32t; i < swapchainImagesCount; ++i)
    {
        createImageView(m_vkImages[i], 1_u32t, m_vkImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_vkImageViews[i]);
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
    VkAttachmentDescription vkColorAttachmentDescription = {};
    vkColorAttachmentDescription.format = m_vkImageFormat;
    vkColorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    vkColorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkColorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkColorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkColorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference vkColorAttachmentReference = {};
    vkColorAttachmentReference.attachment = 0_u32t;
    vkColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription vkDepthAttachmentDescripton = {};
    vkDepthAttachmentDescripton.format = VK_FORMAT_D32_SFLOAT;
    vkDepthAttachmentDescripton.samples = VK_SAMPLE_COUNT_1_BIT;
    vkDepthAttachmentDescripton.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkDepthAttachmentDescripton.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkDepthAttachmentDescripton.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkDepthAttachmentDescripton.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkDepthAttachmentDescripton.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkDepthAttachmentDescripton.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference vkDepthAttachmentReference = {};
    vkDepthAttachmentReference.attachment = 1_u32t;
    vkDepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription vkSubpassDescription = {};
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.colorAttachmentCount = 1_u32t;
    vkSubpassDescription.pColorAttachments = &vkColorAttachmentReference;
    vkSubpassDescription.pDepthStencilAttachment = &vkDepthAttachmentReference;

    std::array<VkAttachmentDescription, 2_u32t> vkAttachmentsDescriptions = {vkColorAttachmentDescription, vkDepthAttachmentDescripton};

    VkSubpassDependency vkSubpassDependency = {};
    vkSubpassDependency.srcSubpass = 0_u32t;
    vkSubpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    vkSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo vkRenderPassCreateInfo = {};
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.attachmentCount = vkAttachmentsDescriptions.size();
    vkRenderPassCreateInfo.pAttachments = vkAttachmentsDescriptions.data();
    vkRenderPassCreateInfo.subpassCount = 1_u32t;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 1_u32t;
    vkRenderPassCreateInfo.pDependencies = &vkSubpassDependency;

    const auto resultCode = vkCreateRenderPass(m_vkDevice, &vkRenderPassCreateInfo, nullptr, &m_vkRenderPass);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createRenderPass(): Function vkCreateRenderPass call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createFramebuffers()
{
    m_vkFramebuffers.resize(static_cast<uint32_t>(m_vkImageViews.size()));
    for (auto i = 0_u64t; i < static_cast<uint64_t>(m_vkImageViews.size()); ++i)
    {
        std::array<VkImageView, 2_u32t> vkImageViews= {m_vkImageViews[i], m_vkDepthImageView};

        VkFramebufferCreateInfo vkFramebufferCreateInfo = {};
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.renderPass = m_vkRenderPass;
        vkFramebufferCreateInfo.attachmentCount = vkImageViews.size();
        vkFramebufferCreateInfo.pAttachments = vkImageViews.data();
        vkFramebufferCreateInfo.width = m_width;
        vkFramebufferCreateInfo.height = m_height;
        vkFramebufferCreateInfo.layers = 1_u32t;

        const auto resultCode = vkCreateFramebuffer(m_vkDevice, &vkFramebufferCreateInfo, nullptr, &m_vkFramebuffers[i]);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::createFramebuffers(): Function vkCreateFramebuffer call failed with a code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
    }
}

void Renderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding vkUniformBufferDescriptorSetLayoutBinding = {};
    vkUniformBufferDescriptorSetLayoutBinding.binding = 0_u32t;
    vkUniformBufferDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkUniformBufferDescriptorSetLayoutBinding.descriptorCount = 1_u32t;
    vkUniformBufferDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding vkSamplerDescriptorSetlayourBinding = {};
    vkSamplerDescriptorSetlayourBinding.binding = 1_u32t;
    vkSamplerDescriptorSetlayourBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkSamplerDescriptorSetlayourBinding.descriptorCount = 1_u32t;
    vkSamplerDescriptorSetlayourBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2_u32t> vkDescriptorSetLayoutBindings = {vkUniformBufferDescriptorSetLayoutBinding, vkSamplerDescriptorSetlayourBinding};

    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {};
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.bindingCount = vkDescriptorSetLayoutBindings.size();
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBindings.data();

    const auto resultCode = vkCreateDescriptorSetLayout(m_vkDevice, &vkDescriptorSetLayoutCreateInfo, nullptr, &m_vkDescriptorSetLayout);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createDescriptorSetLayout(): Function vkCreateDescriptorSetLayout call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
};

void Renderer::createUniformBuffer()
{
    VkDeviceSize vkUniformSize = sizeof(UnifromBufferObject);

    m_vkUniformMatricesBuffers.resize(m_vkImages.size());

    for (auto i = 0_u64t; i < m_vkImages.size(); ++i)
    {
        m_vkUniformMatricesBuffers[i] = m_resourcesMaker->createBuffer(
                     vkUniformSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
    }
};

void Renderer::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2_u32t> vkDescriptorPoolsSizes = {};

    const auto count = static_cast<uint32_t>(m_vkImages.size());

    vkDescriptorPoolsSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolsSizes[0].descriptorCount = count;

    vkDescriptorPoolsSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolsSizes[1].descriptorCount = count;

    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo = {};
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.poolSizeCount = vkDescriptorPoolsSizes.size();
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolsSizes.data();
    vkDescriptorPoolCreateInfo.maxSets = count;

    const auto resultCode = vkCreateDescriptorPool(m_vkDevice, &vkDescriptorPoolCreateInfo, nullptr, &m_vkDescriptorPool);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createDescriptorPool(): Function vkCreateDescriptorPool call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> vkDescriptorSetsLayouts(m_vkImages.size(), m_vkDescriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_vkDescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(m_vkImages.size());
    descriptorSetAllocateInfo.pSetLayouts = vkDescriptorSetsLayouts.data();

    m_vkDescriptorSets.resize(m_vkImages.size());

    const auto resultCode = vkAllocateDescriptorSets(m_vkDevice, &descriptorSetAllocateInfo, m_vkDescriptorSets.data());
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createDescriptorSets(): Function vkAllocateDescriptorSets call failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    for (auto i = 0_u64t; i < m_vkImages.size(); ++i)
    {
        VkDescriptorBufferInfo vkDescriptorBufferInfo = {};
        vkDescriptorBufferInfo.buffer = m_vkUniformMatricesBuffers[i]->getBuffer();
        vkDescriptorBufferInfo.offset = 0_64t;
        vkDescriptorBufferInfo.range = sizeof(UnifromBufferObject);

        VkDescriptorImageInfo vkDescriptorImageInfo = {};
        vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkDescriptorImageInfo.imageView = m_vkTextureImageView;
        vkDescriptorImageInfo.sampler = m_vkTextureSampler;

        std::array<VkWriteDescriptorSet, 2_u32t> vkWriteDescriptorsets = {};

        vkWriteDescriptorsets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorsets[0].dstSet = m_vkDescriptorSets[i];
        vkWriteDescriptorsets[0].dstBinding = 0_u32t;
        vkWriteDescriptorsets[0].dstArrayElement = 0_u32t;
        vkWriteDescriptorsets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vkWriteDescriptorsets[0].descriptorCount = 1_u32t;
        vkWriteDescriptorsets[0].pBufferInfo = &vkDescriptorBufferInfo;

        vkWriteDescriptorsets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorsets[1].dstSet = m_vkDescriptorSets[i];
        vkWriteDescriptorsets[1].dstBinding = 1_u32t;
        vkWriteDescriptorsets[1].dstArrayElement = 0_u32t;
        vkWriteDescriptorsets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        vkWriteDescriptorsets[1].descriptorCount = 1_u32t;
        vkWriteDescriptorsets[1].pImageInfo = &vkDescriptorImageInfo;

        vkUpdateDescriptorSets(m_vkDevice, vkWriteDescriptorsets.size(), vkWriteDescriptorsets.data(), 0_u32t, nullptr);
    }
}

void Renderer::createGraphicsPipeline()
{
    Shader vertexShader("triangle.vert.spv", m_vkDevice);
    Shader fragmentShader("triangle.frag.spv", m_vkDevice);
    if (!vertexShader.load())
    {
        Log::getInstance() << "Renderer::createGraphicsPipeline(): Vertex shader loading error. Terminated." << std::endl;
        std::terminate();
    }
    if (!fragmentShader.load())
    {
        Log::getInstance() << "Renderer:createGraphicsPipeline(): Fragment shader loading error. Terminated." << std::endl;
        std::terminate();
    }

    VkPipelineShaderStageCreateInfo vkVertexShaderStageCreateInfo = {};
    vkVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkVertexShaderStageCreateInfo.module = vertexShader.get();
    vkVertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo vkFragmentShaderStageCreateInfo = {};
    vkFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkFragmentShaderStageCreateInfo.module = fragmentShader.get();
    vkFragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo vkPipelineShadersStagesCreateInfos[] = {vkVertexShaderStageCreateInfo, vkFragmentShaderStageCreateInfo};

    auto vertexBindingDescription = Vertex::getBindingDescription();
    auto vertexAttributesDescriptions = Vertex::getAttributeDescritptions();

    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStageCreateInfo = {};
    vkPipelineVertexInputStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStageCreateInfo.vertexBindingDescriptionCount = 1_u32t;
    vkPipelineVertexInputStageCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vkPipelineVertexInputStageCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributesDescriptions.size());
    vkPipelineVertexInputStageCreateInfo.pVertexAttributeDescriptions = vertexAttributesDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStageCreateInfo = {};
    vkPipelineInputAssemblyStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStageCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vkPipelineInputAssemblyStageCreateInfo.primitiveRestartEnable = VK_FALSE;

//    VkPipelineTessellationStateCreateInfo tessellationInfo = {};
//    tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
//    tessellationInfo

    VkPipelineViewportStateCreateInfo vkPipelineViewportStageCreateInfo = {};
    vkPipelineViewportStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStageCreateInfo.viewportCount = 1_u32t;
    vkPipelineViewportStageCreateInfo.pViewports = &m_vkViewport;
    vkPipelineViewportStageCreateInfo.scissorCount = 1_u32t;
    vkPipelineViewportStageCreateInfo.pScissors = &m_vkRect;

    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStageCreateInfo = {};
    vkPipelineRasterizationStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStageCreateInfo.depthClampEnable = VK_FALSE;
    vkPipelineRasterizationStageCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    vkPipelineRasterizationStageCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStageCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    vkPipelineRasterizationStageCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStageCreateInfo.depthBiasEnable = VK_FALSE;
    vkPipelineRasterizationStageCreateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStageCreateInfo = {};
    vkPipelineMultisampleStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStageCreateInfo.sampleShadingEnable = VK_FALSE;
    vkPipelineMultisampleStageCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStageCreateInfo = {};
    vkPipelineDepthStencilStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStageCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStageCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStageCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    vkPipelineDepthStencilStageCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilStageCreateInfo.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState = {};
    vkPipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
    vkPipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStageCreateInfo = {};
    vkPipelineColorBlendStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStageCreateInfo.logicOpEnable = VK_FALSE;
    vkPipelineColorBlendStageCreateInfo.attachmentCount = 1_u32t;
    vkPipelineColorBlendStageCreateInfo.pAttachments = &vkPipelineColorBlendAttachmentState;

    //VkPipelineDynamicStateCreateInfo dynamicInfo = {};

    VkPipelineLayoutCreateInfo VkPipelineLayoutCreateInfo = {};
    VkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkPipelineLayoutCreateInfo.setLayoutCount = 1_u32t;
    VkPipelineLayoutCreateInfo.pSetLayouts = &m_vkDescriptorSetLayout;

    auto resultCode = vkCreatePipelineLayout(m_vkDevice, &VkPipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createGraphicsPipeline(): Function vkCreatePipelineLayout call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo = {};
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.stageCount = 2_u32t;
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShadersStagesCreateInfos;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStageCreateInfo;
    vkGraphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = m_vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0_u32t;

    resultCode = vkCreateGraphicsPipelines(m_vkDevice, nullptr, 1_u32t, &vkGraphicsPipelineCreateInfo, nullptr, &m_vkPipeline);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createGraphicsPipeline(): Function vkCreateGraphicsPipelines call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createCommandPool()
{
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
    const auto queueFamilies = findQueueFamilies();
    vkCommandPoolCreateInfo.sType  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.queueFamilyIndex = queueFamilies.graphicFamilyIndex.value();

    const auto resultCode = vkCreateCommandPool(m_vkDevice, &vkCommandPoolCreateInfo, nullptr, &m_vkCommandPool);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createCommandPool(): Function vkCreateCommandPool failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createCommandBuffers()
{
    m_vkCommandBuffers.resize(m_vkFramebuffers.size());

    VkCommandBufferAllocateInfo vkCommandBuffersAllocateInfo = {};
    vkCommandBuffersAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBuffersAllocateInfo.commandPool = m_vkCommandPool;
    vkCommandBuffersAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBuffersAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_vkCommandBuffers.size());

    auto resultCode = vkAllocateCommandBuffers(m_vkDevice, &vkCommandBuffersAllocateInfo, m_vkCommandBuffers.data());
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createCommandBuffers(): Function vkAllocateCommandBuffers call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    for (auto i = 0_u64t; i < m_vkCommandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo = {};
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        resultCode = vkBeginCommandBuffer(m_vkCommandBuffers[i], &vkCommandBufferBeginInfo);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::createCommandBuffers(): Function vkBeginCommandBuffer call failed with a code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }

        VkRenderPassBeginInfo vkRenderPassBeginInfo = {};
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.renderPass = m_vkRenderPass;
        vkRenderPassBeginInfo.framebuffer = m_vkFramebuffers[i];
        vkRenderPassBeginInfo.renderArea.offset = {0_32t, 0_32t};
        vkRenderPassBeginInfo.renderArea.extent = { m_width, m_height};

        std::array<VkClearValue, 2_u32t> vkClearValues;

        vkClearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        vkClearValues[1].depthStencil = {1.0f, 0};
        vkRenderPassBeginInfo.clearValueCount = vkClearValues.size();
        vkRenderPassBeginInfo.pClearValues = vkClearValues.data();

        VkDeviceSize vkOffsetsSizes[] = {0_u32t};
        vkCmdBeginRenderPass(m_vkCommandBuffers[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
            vkCmdBindVertexBuffers(m_vkCommandBuffers[i], 0_u32t, 1_u32t, &m_vkVertexBuffer->getBuffer(), vkOffsetsSizes);
            vkCmdBindIndexBuffer(m_vkCommandBuffers[i], m_vkIndexBuffer->getBuffer(), 0_u32t, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0_u32t, 1_u32t, &m_vkDescriptorSets[i],  0_u32t, nullptr);
            vkCmdDrawIndexed(m_vkCommandBuffers[i], m_model->getIndicesCount(), 1_u32t, 0_u32t, 0_u32t, 0_u32t);
        vkCmdEndRenderPass(m_vkCommandBuffers[i]);

        resultCode = vkEndCommandBuffer(m_vkCommandBuffers[i]);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::createCommandBuffers(): Function vkEndCommandBuffer call failed with a code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
    }
}

void Renderer::createSyncObjects()
{
    m_vkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkInFlightFramesFence.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo vkSemaphoreCreateInfo = {};
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo vkFenceCreateInfo = {};
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = 0_u64t; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (VK_SUCCESS != vkCreateSemaphore(m_vkDevice, &vkSemaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphores[i]) ||
            VK_SUCCESS != vkCreateSemaphore(m_vkDevice, &vkSemaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphores[i]) ||
            VK_SUCCESS != vkCreateFence(m_vkDevice, &vkFenceCreateInfo, nullptr, &m_vkInFlightFramesFence[i]))
        {
            Log::getInstance() << "Failed to create synchronization objects. Terminated." << std::endl;
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
    createDepthResources();
    createFramebuffers();
    createCommandBuffers();
}

void Renderer::createVertexBuffer()
{
    VkDeviceSize vkBufferSize = m_model->getVerticiesSizeForBuffer();

    auto stagingBuffer = m_resourcesMaker->createBuffer(
                             vkBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* bufferMemory;
    vkMapMemory(m_vkDevice, stagingBuffer->getBufferMemory(), 0_u64t, vkBufferSize, 0_u32t, &bufferMemory);
    memcpy(bufferMemory, m_model->getVerticesData(), vkBufferSize);
    vkUnmapMemory(m_vkDevice, stagingBuffer->getBufferMemory());

    m_vkVertexBuffer = m_resourcesMaker->createBufferCopy(stagingBuffer,
                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Renderer::createIndexBuffer()
{
    VkDeviceSize vkBufferSize = m_model->getVerticiesIndexesSizeForBuffer();

    auto stagingBuffer = m_resourcesMaker->createBuffer(
                             vkBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *bufferMemory;
    vkMapMemory(m_vkDevice, stagingBuffer->getBufferMemory(), 0_u64t, vkBufferSize, 0_u32t, &bufferMemory);
    memcpy(bufferMemory, m_model->getVerticiesIndicesData(), vkBufferSize);
    vkUnmapMemory(m_vkDevice, stagingBuffer->getBufferMemory());

    m_vkIndexBuffer = m_resourcesMaker->createBufferCopy(
                          stagingBuffer,
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

uint32_t Renderer::findMemoryType(uint32_t requiredTypes, VkMemoryPropertyFlags requiredProperties)
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
    vkMapMemory(m_vkDevice, m_vkUniformMatricesBuffers[swapchainImageIndex]->getBufferMemory(), 0_u64t, sizeof(UnifromBufferObject), 0_u64t, &uniformData);
    memcpy(uniformData, &mvpMatrix, sizeof(UnifromBufferObject));
    vkUnmapMemory(m_vkDevice, m_vkUniformMatricesBuffers[swapchainImageIndex]->getBufferMemory());
}

void Renderer::createImage(uint32_t width,
                 uint32_t height,
                 uint32_t mipLevels,
                 VkFormat format,
                 VkImageTiling tiling,
                 VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties,
                 VkImage &image,
                 VkDeviceMemory &imageMemory)
{
    VkImageCreateInfo vkImageCreateInfo = {};
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = format;
    vkImageCreateInfo.extent.width = width;
    vkImageCreateInfo.extent.height = height;
    vkImageCreateInfo.extent.depth = 1_u32t;
    vkImageCreateInfo.mipLevels = mipLevels;
    vkImageCreateInfo.arrayLayers = 1_u32t;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = tiling;
    vkImageCreateInfo.usage = usage;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    auto resultCode = vkCreateImage(m_vkDevice, &vkImageCreateInfo, nullptr, &image);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createImage(): Function vkCreateImage call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    VkMemoryRequirements vkMemoryRequirements;
    vkGetImageMemoryRequirements(m_vkDevice, image, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMemoryAllocateInfo = {};
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = findMemoryType(vkMemoryRequirements.memoryTypeBits, properties);

    resultCode = vkAllocateMemory(m_vkDevice, &vkMemoryAllocateInfo, nullptr, &imageMemory);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createImage(): Function vkAllocateMemory call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    vkBindImageMemory(m_vkDevice, image, imageMemory, 0_u64t);
}

void Renderer::createTextureImage()
{
    IO::TgaLoader texturesLoader;
    texturesLoader.loadFile("texture.tga");
    auto texture = texturesLoader.generateTexture();

    Log::getInstance() << m_vkTextureImageMipLevels << " mip levels" << std::endl;
    if (!texture)
    {
        Log::getInstance() << "Renderer::createTextureImage(): Failed to load texture image." << std::endl;
        std::terminate();
    }

    const auto textureWidth = texture->getWidth();
    const auto imageSize = texture->getImageSize();
    const auto textureHeight = texture->getHeight();
    void *pixelsData = texture->getImageData();

    m_vkTextureImageMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1_u32t;

    auto stagingBuffer = m_resourcesMaker->createBuffer(
                             imageSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void *stagingBufferData = nullptr;
    vkMapMemory(m_vkDevice, stagingBuffer->getBufferMemory(), 0_u64t, imageSize, 0_u64t, &stagingBufferData);
    memcpy(stagingBufferData, pixelsData, imageSize);
    vkUnmapMemory(m_vkDevice, stagingBuffer->getBufferMemory());

    createImage(static_cast<uint32_t>(textureWidth),
                static_cast<uint32_t>(textureHeight),
                m_vkTextureImageMipLevels,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vkTextureImage,
                m_vkTextureImageMemory);

    transitionImageLayout(m_vkTextureImage, 1_u32t, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer->getBuffer(), m_vkTextureImage, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight));
    //transitionImageLayout(m_vkTextureImage, 1_u32t, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    generateMipmaps(m_vkTextureImage, textureWidth, textureHeight, m_vkTextureImageMipLevels);
}

void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    auto commandBuffer = m_resourcesMaker->createSingleTimeCommandBuffer();

    VkBufferImageCopy vkBufferImageCopyRegion = {};
    vkBufferImageCopyRegion.bufferOffset = 0_u32t;
    vkBufferImageCopyRegion.bufferRowLength = 0_u32t;
    vkBufferImageCopyRegion.bufferImageHeight = 0_u32t;
    vkBufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopyRegion.imageSubresource.mipLevel = 0_u32t;
    vkBufferImageCopyRegion.imageSubresource.baseArrayLayer = 0_u32t;
    vkBufferImageCopyRegion.imageSubresource.layerCount = 1_u32t;
    vkBufferImageCopyRegion.imageOffset = {0_32t, 0_32t, 0_32t};
    vkBufferImageCopyRegion.imageExtent.width = width;
    vkBufferImageCopyRegion.imageExtent.height = height;
    vkBufferImageCopyRegion.imageExtent.depth = 1_u32t;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1_u32t, &vkBufferImageCopyRegion);
    commandBuffer.submit();
}

void Renderer::transitionImageLayout(VkImage image, uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    auto vkCommandBuffer = m_resourcesMaker->createSingleTimeCommandBuffer();

    VkImageMemoryBarrier vkImageMemoryBarrier = {};
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.oldLayout = oldLayout;
    vkImageMemoryBarrier.newLayout = newLayout;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = image;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0_u32t;
    vkImageMemoryBarrier.subresourceRange.levelCount = mipLevels;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0_u32t;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1_u32t;

    VkPipelineStageFlags vkPipelineSourceStage;
    VkPipelineStageFlags vkPipelineDestinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0_u32t;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkPipelineSourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineDestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkPipelineSourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineDestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0_u32t;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        vkPipelineSourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineDestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        Log::getInstance() << "Renderer::transitionImageLayout(): Failed to load texture image." << std::endl;
        std::terminate();
    }

    vkCmdPipelineBarrier(vkCommandBuffer, vkPipelineSourceStage, vkPipelineDestinationStage, 0_u32t, 0_u32t, nullptr, 0_u32t, nullptr, 1_u32t, &vkImageMemoryBarrier);
    vkCommandBuffer.submit();
}

void Renderer::createImageView(VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags imageAspectFlags, VkImageView &imageView)
{
    VkImageViewCreateInfo vkImageViewCreateInfo = {};
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.image = image;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format = format;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    vkImageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0_u32t;
    vkImageViewCreateInfo.subresourceRange.levelCount = mipLevels;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0_u32t;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1_u32t;


    const auto resultCode = vkCreateImageView(m_vkDevice, &vkImageViewCreateInfo, nullptr, &imageView);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createImageView(): Failed to load texture image view." << std::endl;
        std::terminate();
    }
}

void Renderer::createTextureImageView()
{
    createImageView(m_vkTextureImage, m_vkTextureImageMipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_vkTextureImageView);
}

void Renderer::createTextureSampler()
{
    VkSamplerCreateInfo vkSamplerCreateInfo = {};
    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.mipLodBias = 0.0f;
    vkSamplerCreateInfo.minLod = 0.0f;
    vkSamplerCreateInfo.maxLod = static_cast<float>(m_vkTextureImageMipLevels);
    vkSamplerCreateInfo.anisotropyEnable = VK_TRUE;
    vkSamplerCreateInfo.maxAnisotropy = 16_u32t;
    vkSamplerCreateInfo.compareEnable = VK_FALSE;
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    const auto resultCode = vkCreateSampler(m_vkDevice, &vkSamplerCreateInfo, nullptr, &m_vkTextureSampler);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createTextureSampler(): Function vkCreateImageView call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createDepthResources()
{
    // VkFormat format = findDepthFormat(); in vulkan-tutorial.com guide
    VkFormat vkDepthFormat =  VK_FORMAT_D32_SFLOAT;
    createImage(m_width,
                m_height,
                1_u32t,
                vkDepthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vkDepthImage,
                m_vkDepthImageMemory);
    createImageView(m_vkDepthImage, 1_u32t, vkDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, m_vkDepthImageView);
    transitionImageLayout(m_vkDepthImage, 1_u32t, VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::generateMipmaps(VkImage image, int32_t imageWidth, int32_t imageHeight, uint32_t mipLevelsCount)
{
    auto commandBuffer = m_resourcesMaker->createSingleTimeCommandBuffer();

    VkImageMemoryBarrier vkImageMemoryBarrier = {};
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.image = image;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0_u32t;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1_u32t;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1_u32t;

    int32_t mipWidth = imageWidth;
    int32_t mipHeight = imageHeight;

    for (auto i = 1_u32t; i < mipLevelsCount; ++i)
    {
        vkImageMemoryBarrier.subresourceRange.baseMipLevel = i - 1_u32t;
        vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0_u32t,
                             0_u32t, nullptr, 0_u32t, nullptr, 1_u32t, &vkImageMemoryBarrier);

        VkImageBlit vkImageBlit = {};
        vkImageBlit.srcOffsets[0] = { 0_32t, 0_32t, 0_32t };
        vkImageBlit.srcOffsets[1] = { mipWidth, mipHeight, 1_32t };
        vkImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vkImageBlit.srcSubresource.mipLevel = i - 1_u32t;
        vkImageBlit.srcSubresource.baseArrayLayer = 0_u32t;
        vkImageBlit.srcSubresource.layerCount = 1_u32t;
        vkImageBlit.dstOffsets[0] = { 0_32t, 0_32t, 0_32t };
        vkImageBlit.dstOffsets[1] = { mipWidth > 1_32t ? mipWidth / 2_32t : 1_32t, mipHeight > 1_32t ? mipHeight / 2_32t : 1_32t, 1_32t };
        vkImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vkImageBlit.dstSubresource.mipLevel = i;
        vkImageBlit.dstSubresource.baseArrayLayer = 0_u32t;
        vkImageBlit.dstSubresource.layerCount = 1_u32t;

        vkCmdBlitImage(commandBuffer,
                       image,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1_u32t,
                       &vkImageBlit,
                       VK_FILTER_LINEAR);

        vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
                    commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0_u32t,
                    0_u32t, nullptr,
                    0_u32t, nullptr,
                    1_u32t, &vkImageMemoryBarrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    vkImageMemoryBarrier.subresourceRange.baseMipLevel = mipLevelsCount - 1;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0_u32t,
        0_u32t, nullptr,
        0_u32t, nullptr,
        1_u32t, &vkImageMemoryBarrier);
    commandBuffer.submit();
}
