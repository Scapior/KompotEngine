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

QueueFamilyIndices KompotEngine::Renderer::findQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
{
    QueueFamilyIndices indices;

    auto queueFamiliesCount = 0_u32t;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamiliesCount, queueFamilies.data());

    for ( auto i = 0_u32t; i < queueFamilies.size(); ++i)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);

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
        log << "Not all queue families found. Terminated." << std::endl;
        std::terminate();
    }

    return indices;
}

void KompotEngine::Renderer::createLogicalDeviceAndQueue(VulkanDevice &vulkanDevice, VkSurfaceKHR vkSurface)
{
    //VkQueueFamilyProperties
    const auto familiesIndecies = findQueueFamilies(vulkanDevice.physicalDevice, vkSurface);
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

    const auto resultCode = vkCreateDevice(vulkanDevice.physicalDevice, &deviceInfo, nullptr, &vulkanDevice.device);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateDevice failed. Terminated." << std::endl;
        std::terminate();
    }

    vkGetDeviceQueue(vulkanDevice.device, familiesIndecies.graphicFamilyIndex.value(), 0_u32t, &vulkanDevice.graphicQueue);
    vkGetDeviceQueue(vulkanDevice.device, familiesIndecies.presentFamilyIndex.value(), 0_u32t, &vulkanDevice.presentQueue);
}

void KompotEngine::Renderer::createVulkanDevice(VkInstance vkInstance, VkSurfaceKHR vkSurface, VulkanDevice &vulkanDevice)
{
    selectPhysicalDevice(vkInstance, vulkanDevice.physicalDevice);
    createLogicalDeviceAndQueue(vulkanDevice, vkSurface);
}

void KompotEngine::Renderer::createSurface(VkInstance vkInstance, GLFWwindow *window, VkSurfaceKHR &vkSurface)
{
    const auto resultCode = glfwCreateWindowSurface(vkInstance, window, nullptr, &vkSurface);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "glfwCreateWindowSurface failed. Terminated" << std::endl;
        std::terminate();
    }
}

SwapchainSupportDetails KompotEngine::Renderer::getSwapchainDetails(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
{
    SwapchainSupportDetails swapchainDetails;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &swapchainDetails.capabilities);

    auto formatsCount = 0_u32t;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatsCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatsCount, formats.data());
    swapchainDetails.format = chooseSurfaceFormat(formats);


    auto presentModesCount = 0_u32t;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModesCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModesCount, presentModes.data());
    swapchainDetails.presentMode = choosePresentMode(presentModes);

    return swapchainDetails;
}

VkSurfaceFormatKHR KompotEngine::Renderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats)
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

VkExtent2D KompotEngine::Renderer::chooseExtent(const VkSurfaceCapabilitiesKHR &vkSurfaceCapabilities, uint32_t width, uint32_t height)
{
    if (vkSurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return vkSurfaceCapabilities.currentExtent;
    }
    else
    {
        VkExtent2D extent = {width, height};
        extent.width = std::clamp(extent.width, vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, vkSurfaceCapabilities.minImageExtent.height, vkSurfaceCapabilities.maxImageExtent.height);
        return extent;
    }
}

VkPresentModeKHR KompotEngine::Renderer::choosePresentMode(const std::vector<VkPresentModeKHR> &presentModes)
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

void KompotEngine::Renderer::createSwapchain(
        const VulkanDevice &vulkanDevice,
        VkSurfaceKHR vkSurface,
        uint32_t width,
        uint32_t height,
        VulkanSwapchain &vulkanSwapchain)
{
    Log &log = Log::getInstance();
    vulkanSwapchain.setDevice(vulkanDevice.device);
    const auto swapchainDetails = getSwapchainDetails(vulkanDevice.physicalDevice, vkSurface);
    const auto queuefamilies = findQueueFamilies(vulkanDevice.physicalDevice, vkSurface);
    uint32_t queuefamiliesIndicies[] = {
        queuefamilies.graphicFamilyIndex.value(),
        queuefamilies.presentFamilyIndex.value()
    };

    vulkanSwapchain.imageExtent = chooseExtent(swapchainDetails.capabilities, width, height);
    vulkanSwapchain.imageFormat = swapchainDetails.format.format;

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = vkSurface;
    swapchainInfo.minImageCount = swapchainDetails.capabilities.minImageCount + 1_u32t;
    swapchainInfo.imageFormat = swapchainDetails.format.format;
    swapchainInfo.imageColorSpace = swapchainDetails.format.colorSpace;
    swapchainInfo.imageExtent = chooseExtent(swapchainDetails.capabilities, width, height);
    swapchainInfo.imageArrayLayers = 1_u32t;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    //swapchainInfo.1
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

    const auto resultCode = vkCreateSwapchainKHR(vulkanDevice.device, &swapchainInfo, nullptr, &vulkanSwapchain.swapchain);
    if (resultCode != VK_SUCCESS)
    {
        log << "vkCreateSwapchainKHR failed. Terminated." << std::endl;
        std::terminate();
    }
    auto swapchainImagesCount = 0_u32t;
    vkGetSwapchainImagesKHR(vulkanDevice.device, vulkanSwapchain.swapchain, &swapchainImagesCount, nullptr);
    vulkanSwapchain.images.resize(swapchainImagesCount);
    vkGetSwapchainImagesKHR(vulkanDevice.device, vulkanSwapchain.swapchain, &swapchainImagesCount, vulkanSwapchain.images.data());

    vulkanSwapchain.imagesViews.resize(swapchainImagesCount);
    for (auto i = 0_u32t; i < swapchainImagesCount; ++i)
    {
        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = vulkanSwapchain.images[i];
        imageViewInfo.format = vulkanSwapchain.imageFormat;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.layerCount     = 1_u32t;
        imageViewInfo.subresourceRange.levelCount     = 1_u32t;
        imageViewInfo.subresourceRange.baseMipLevel   = 0_u32t;
        imageViewInfo.subresourceRange.baseArrayLayer = 0_u32t;

        const auto resultCode = vkCreateImageView(vulkanDevice.device, &imageViewInfo, nullptr, &vulkanSwapchain.imagesViews[i]);
        if (resultCode != VK_SUCCESS)
        {
            log << "vkCreateSwapchainKHR failed (i = " << i  << "). Terminated." << std::endl;
            std::terminate();
        }
    }
    vulkanSwapchain.viewport.x = 0.0f;
    vulkanSwapchain.viewport.y = 0.0f;
    vulkanSwapchain.viewport.height = static_cast<float>(vulkanSwapchain.imageExtent.height);
    vulkanSwapchain.viewport.width = static_cast<float>(vulkanSwapchain.imageExtent.width);
    vulkanSwapchain.viewport.minDepth = 0.0f;
    vulkanSwapchain.viewport.maxDepth = 1.0f;

    vulkanSwapchain.scissorRect.offset = {0_u32t, 0_u32t};
    vulkanSwapchain.scissorRect.extent = vulkanSwapchain.imageExtent;
}

void KompotEngine::Renderer::createRenderPass(
        VkDevice device,
        const VulkanSwapchain &swapchain,
        VkRenderPass &renderPass)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchain.imageFormat;
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


    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1_u32t;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1_u32t;
    renderPassInfo.pSubpasses = &subpassDescription;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateRenderPass failed. Terminated." << std::endl;
        std::terminate();
    }
}

void KompotEngine::Renderer::createFramebuffers(VkDevice device, VkRenderPass renderPass, VulkanSwapchain &swapchain)
{
    Log &log = Log::getInstance();
    swapchain.framebuffers.resize(static_cast<uint32_t>(swapchain.imagesViews.size()));
    for (auto i = 0_u64t; i < static_cast<uint64_t>(swapchain.imagesViews.size()); ++i)
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1_u32t;//static_cast<uint32_t>(swapchain.imagesViews.size());
        framebufferInfo.pAttachments = &swapchain.imagesViews[i];
        framebufferInfo.width = swapchain.imageExtent.width;
        framebufferInfo.height = swapchain.imageExtent.height;
        framebufferInfo.layers = 1_u32t;

        const auto result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchain.framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            log << "vkCreateFramebuffer failed. Terminated." << std::endl;
            std::terminate();
        }
    }


}

void KompotEngine::Renderer::createGraphicsPipeline(
        VkDevice device,
        VulkanSwapchain &vulkanSwapchain,
        VkRenderPass renderPass,
        VulkanPipeline &pipleline)
{
    Log &log = Log::getInstance();
    Shader vertexShader("triangle.vert.spv", device);
    Shader fragmentShader("triangle.frag.spv", device);
    if (!vertexShader.load())
    {
        log << "Vertex shader loading error. Terminated." << std::endl;
    }
    if (!fragmentShader.load())
    {
        log << "Fragment shader loading error. Terminated." << std::endl;
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

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0_u32t;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0_u32t;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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
    viewportInfo.pViewports = &vulkanSwapchain.viewport;
    viewportInfo.scissorCount = 1_u32t;
    viewportInfo.pScissors = &vulkanSwapchain.scissorRect;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipleline.pipelineLayout) != VK_SUCCESS)
    {
        log << "vkCreatePipelineLayout failed. Terminated." << std::endl;
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
    pipelineCreateInfo.layout = pipleline.pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0_u32t;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1_u32t, &pipelineCreateInfo, nullptr, &pipleline.pipeline) != VK_SUCCESS)
    {
        log << "vkCreateGraphicsPipelines failed. Terminated." << std::endl;
        std::terminate();
    }
}
