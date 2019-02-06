#pragma once

#include "global.hpp"
#include "Shader.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <set>

namespace KompotEngine
{

namespace Renderer
{

struct VulkanPipeline
{
    VkPipelineLayout pipelineLayout;
    VkPipeline       pipeline;
    void setDevice(VkDevice device)
    {
        m_device = device;
    }
    void destroy()
    {
        vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
        vkDestroyPipeline(m_device, pipeline, nullptr);
    }
private:
    VkDevice m_device;
};

struct VulkanDevice
{
    VkPhysicalDevice physicalDevice; // will be implicitly destroyed with VkInstance
    VkDevice         device;

    VkQueue graphicQueue;
    VkQueue presentQueue;

    void destroy()
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

    VkViewport viewport;
    VkRect2D   scissorRect;

    std::vector<VkFramebuffer> framebuffers;

    void setDevice(VkDevice device)
    {
        m_device = device;
    }
    void destroy()
    {
        for (auto &framebuffer : framebuffers)
        {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }

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
#ifdef _DEBUG
    "VK_LAYER_LUNARG_standard_validation"
#endif
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
void createRenderPass(VkDevice, const VulkanSwapchain&, VkRenderPass&);
void createFramebuffers(VkDevice, VkRenderPass, VulkanSwapchain&);
void createGraphicsPipeline(VkDevice, VulkanSwapchain&, VkRenderPass, VulkanPipeline&);
void createCommandPool(VulkanDevice, VkSurfaceKHR, VkCommandPool&);
void createCommandBuffers(VkDevice, VkCommandPool, VkRenderPass, const std::vector<VkFramebuffer>&, VkExtent2D, VkPipeline, std::vector<VkCommandBuffer>&);
void createSyncObjects(VkDevice, uint64_t, std::vector<VkSemaphore>&, std::vector<VkSemaphore>&, std::vector<VkFence>&);

} // namespace Renderer

} //namespace KompotEngine
