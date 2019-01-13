#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace KompotEngine
{

namespace Renderer
{

class Renderer
{
public:
    Renderer(GLFWwindow*, uint32_t, uint32_t);
    void run();
private:
    GLFWwindow *m_glfwWindowHandler;
    uint32_t    m_screenWidth;
    uint32_t    m_screenHeight;
};


} // namespace Renderer

} //namespace KompotEngine
