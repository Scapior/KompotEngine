#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>

namespace KompotEngine
{

namespace Renderer
{

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicIndex;
    std::optional<uint32_t> tranferIndex;

    bool isComplete() const
    {
        return graphicIndex.has_value();
    }

};

static PFN_vkCreateDebugUtilsMessengerEXT  pfn_vkCreateDebugUtilsMessengerEXT  = nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;

void createVkInstance(VkInstance&, const std::string&);
void loadFuntions(VkInstance);
void setupDebugCallback(VkInstance, VkDebugUtilsMessengerEXT&);
void selectPhysicalDevice(VkInstance, VkPhysicalDevice&);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);
void createLogicalDeviceAndQueue(VkPhysicalDevice, VkDevice&, VkQueue&);

std::vector<std::string> getLayers();
std::vector<std::string> getExtensions();

} // namespace Renderer

} //namespace KompotEngine
