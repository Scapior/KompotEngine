#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <algorithm>


namespace KompotEngine
{

namespace Renderer
{

static PFN_vkCreateDebugUtilsMessengerEXT  pfn_vkCreateDebugUtilsMessengerEXT  = nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;

void createVkInstance(VkInstance&, const std::string&);
void loadFuntions(VkInstance&);
void setupDebugCallback(VkInstance&, VkDebugUtilsMessengerEXT&);
void selectPhysicalDevice(VkInstance&, VkPhysicalDevice&);

std::vector<std::string> getLayers();
std::vector<std::string> getExtensions();

} // namespace Renderer

} //namespace KompotEngine
