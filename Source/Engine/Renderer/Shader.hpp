#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

namespace KompotEngine
{

namespace Renderer
{

class Shader
{
public:
    Shader(const std::string&, VkDevice device);
    ~Shader();

    bool load();
    VkShaderModule get() const;
private:
    std::string       m_filename;
    VkDevice          m_device;
    VkShaderModule    m_shaderModule;
};

} // namespace Renderer

} //namespace KompotEngine
