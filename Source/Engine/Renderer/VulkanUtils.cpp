#include "VulkanUtils.hpp"

using namespace  KompotEngine::Renderer;

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
        log << "Vulkan layer \"" << availablelayer.layerName  << "\" has found." << std::endl;
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
        const auto& requiredExtensionName = glfwRequiredExtensions[i];
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

    for (const auto& extensionProperty : availableVulkanExtensions)
    {
        log << "Vulkan extension \"" << extensionProperty.extensionName  << "\" has found." << std::endl;
        extensions.push_back(extensionProperty.extensionName);
    }
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}
