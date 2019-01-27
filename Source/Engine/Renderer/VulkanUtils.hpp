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

struct VulkanDevice
{
    VkPhysicalDevice physicalDevice; // will be implicitly destroyed with VkInstance
    VkDevice         device;

    VkQueue graphicQueue;
    VkQueue presentQueue;

    ~VulkanDevice()
    {
        vkDestroyDevice(device, nullptr);
    }
};

struct VulkanSwapchain
{
    VkSwapchainKHR           swapchain;
    std::vector<VkImage>     images;
    std::vector<VkImageView> imagesViews;

    VkFormat   imageFormat;
    VkExtent2D imageExtent;

    void setDevice(VkDevice device)
    {
        m_device = device;
    }
    ~VulkanSwapchain()
    {
        vkDestroySwapchainKHR(m_device, swapchain, nullptr);
        for (auto &imageView : imagesViews)
        {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
    }
private:
    VkDevice m_device;
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR       format;
    VkPresentModeKHR         presentMode;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;

    bool isComplete() const
    {
        return graphicFamilyIndex.has_value() &&
               presentFamilyIndex.has_value();
    }

};

static PFN_vkCreateDebugUtilsMessengerEXT  pfn_vkCreateDebugUtilsMessengerEXT  = nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;

static std::vector<const char*> validationLayers {
    "VK_LAYER_LUNARG_standard_validation"
};

void createVkInstance(VkInstance&, const std::string&);
void loadFuntions(VkInstance);
void setupDebugCallback(VkInstance, VkDebugUtilsMessengerEXT&);
void createSurface(VkInstance, GLFWwindow*, VkSurfaceKHR&);
void createVulkanDevice(VkInstance, VkSurfaceKHR, VulkanDevice&);
void selectPhysicalDevice(VkInstance, VkPhysicalDevice&);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);
void createLogicalDeviceAndQueue(VulkanDevice&, VkSurfaceKHR);
SwapchainSupportDetails getSwapchainDetails(VkPhysicalDevice, VkSurfaceKHR);
VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>&);
VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR&, uint32_t, uint32_t);
void createSwapchain(const VulkanDevice&, VkSurfaceKHR, uint32_t, uint32_t, VulkanSwapchain&);


} // namespace Renderer

} //namespace KompotEngine
