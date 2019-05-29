#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <set>

namespace KompotEngine
{

namespace Renderer
{

struct QueueFamilyIndices;

class Device
{
public:
    Device(const std::string&);
    ~Device();

    VkResult create(VkSurfaceKHR);

    VkInstance getInstance() const;

    operator VkPhysicalDevice () const;
    operator VkDevice () const;

    VkQueue getGraphicsQueue() const;

    QueueFamilyIndices findQueueFamilies();
    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);


private:
    std::string m_applicationName;

    VkInstance       m_vkInstance;
    VkPhysicalDevice m_vkPhysicalDevice;
    VkDevice         m_vkDevice;

    VkSurfaceKHR     m_vkSurface;

    VkQueue m_vkGraphicsQueue;
    VkQueue m_vkPresentQueue;

    VkResult createVkInstance();
    VkResult selectPhysicalDevice();

    VkResult createDevice();

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

static const std::vector<const char*> validationLayers {
#ifdef ENGINE_DEBUG
    "VK_LAYER_LUNARG_standard_validation"
#endif
};

} // namespace Renderer

} //namespace KompotEngine
