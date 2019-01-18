#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

Renderer::Renderer(GLFWwindow *window, uint32_t width, uint32_t height, const std::string &windowName)
    : m_glfwWindowHandler(window), m_screenWidth(width), m_screenHeight(height), m_vkInstance(nullptr)
{
    Log &log = Log::getInstance();

    // VkInstance

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = windowName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0_u8t, 0_u8t, 1_u8t);
    applicationInfo.pEngineName = ENGINE_NAME.c_str();
    applicationInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VESRION_MAJOR, ENGINE_VESRION_MINOR, ENGINE_VESRION_PATCH);
    applicationInfo.apiVersion = VK_API_VERSION_1_1;

    auto layersStrings = getLayers();
    std::vector<const char*> layers(layersStrings.size());
    std::transform(std::begin(layersStrings), std::end(layersStrings), std::begin(layers),
                   [&](const auto& layerString)
    {
        return layerString.c_str();
    });

    auto extensionsStrings = getExtensions();
    std::vector<const char*> extensions(extensionsStrings.size());
    std::transform(std::begin(extensionsStrings), std::end(extensionsStrings), std::begin(extensions),
                   [&](const auto& extensionString)
    {
        return extensionString.c_str();
    });

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = static_cast<unsigned int>(layers.size());
    instanceInfo.ppEnabledLayerNames = layers.data();
    instanceInfo.enabledExtensionCount = static_cast<unsigned int>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&instanceInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
    {
        log << "vkCreateInstance failed. Terminated." << std::endl;
        std::terminate();
    }
}

Renderer::~Renderer()
{
    vkDestroyInstance(m_vkInstance, nullptr);
}

void Renderer::run()
{
    while (!glfwWindowShouldClose(m_glfwWindowHandler)) // todo: remove all this
    {
        //glfwSwapBuffers(m_glfwWindowHandler);
        glfwPollEvents();
    }
}
