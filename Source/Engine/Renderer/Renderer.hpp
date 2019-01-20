#pragma once

#include "global.hpp"
#include "VulkanUtils.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>

namespace KompotEngine
{

namespace Renderer
{

class Renderer
{
public:
    Renderer(GLFWwindow*, uint32_t, uint32_t, const std::string&);
    void run();
    ~Renderer();
private:
    // window
    GLFWwindow *m_glfwWindowHandler;
    uint32_t    m_screenWidth;
    uint32_t    m_screenHeight;

    // vulkan members
    VkInstance       m_vkInstance;
    VkPhysicalDevice m_vkPhysicalDevice; // will be implicitly destroyed with VkInstance
    VkDevice         m_vkDevice;

    VkQueue          m_vkGraphicQueue;

    VkDebugUtilsMessengerEXT m_vkDebugMessenger;

};


} // namespace Renderer

} //namespace KompotEngine
