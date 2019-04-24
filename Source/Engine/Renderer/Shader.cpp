#include "Shader.hpp"

using namespace  KompotEngine::Renderer;

KompotEngine::Renderer::Shader::Shader(const std::string &filename, VkDevice device)
    : m_filename(filename), m_device(device)
{}

KompotEngine::Renderer::Shader::~Shader()
{
    vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
}

bool KompotEngine::Renderer::Shader::load()
{
    std::ifstream file("Shaders/"s + m_filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Log::getInstance() << "Couldn't open shader file " << m_filename << std::endl;
        return false;
    }
    const auto fileSize = file.tellg();
    std::vector<char> buffer(static_cast<unsigned long>(fileSize));
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo = {};
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.codeSize = static_cast<unsigned long>(fileSize);
    vkShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    return VK_SUCCESS == vkCreateShaderModule(m_device, &vkShaderModuleCreateInfo, nullptr, &m_shaderModule);

}

VkShaderModule KompotEngine::Renderer::Shader::get() const
{
    return m_shaderModule;
}
