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
        Log &log = Log::getInstance();
        log << "Couldn't open shader file " << m_filename << std::endl;
        return false;
    }
    const auto fileSize = file.tellg();
    std::vector<char> buffer(static_cast<unsigned long>(fileSize));
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo shaderInfo = {};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = static_cast<unsigned long>(fileSize);
    shaderInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    const auto resultCode = vkCreateShaderModule(m_device, &shaderInfo, nullptr, &m_shaderModule);
    return resultCode == VK_SUCCESS;
}

VkShaderModule KompotEngine::Renderer::Shader::get() const
{
    return m_shaderModule;
}
