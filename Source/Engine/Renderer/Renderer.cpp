#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

Renderer::Renderer(GLFWwindow *window, uint32_t width, uint32_t height, const std::string &windowName)
    : m_glfwWindowHandler(window), m_screenWidth(width), m_screenHeight(height)
{
    createVkInstance(m_vkInstance, windowName);
    loadFuntions(m_vkInstance);
    setupDebugCallback(m_vkInstance, m_vkDebugMessenger);
    createSurface(m_vkInstance, m_glfwWindowHandler, m_vkSurface);
    createVulkanDevice(m_vkInstance, m_vkSurface, m_device);
    createSwapchain(m_device, m_vkSurface, width, height, m_swapchain);
    createRenderPass(m_device.device, m_swapchain, m_renderPass);
    createFramebuffers(m_device.device, m_renderPass, m_swapchain);
    createGraphicsPipeline(m_device.device, m_swapchain, m_renderPass, m_pipeline);
    createCommandPool(m_device, m_vkSurface, m_commandPool);
    createCommandBuffers(m_device.device, m_commandPool, m_renderPass, m_swapchain.framebuffers, m_swapchain.imageExtent, m_pipeline.pipeline, m_commandBuffers);
    createSyncObjects(m_device.device, MAX_FRAMES_IN_FLIGHT , m_imageAvailableSemaphores, m_renderFinishedSemaphores, m_inFlightFramesFence);
}

Renderer::~Renderer()
{
    for (auto i = 0_u64t; i < MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        vkDestroySemaphore(m_device.device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_device.device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device.device, m_inFlightFramesFence[i], nullptr);
    }
    vkDestroyCommandPool(m_device.device, m_commandPool, nullptr);
    m_pipeline.destroy();
    vkDestroyRenderPass(m_device.device, m_renderPass, nullptr);
    m_swapchain.destroy();
    m_device.destroy();
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
    pfn_vkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);
}

void Renderer::run()
{
    Log &log = Log::getInstance();
    auto currentFrameIndex = 0_u64t;
    while (!glfwWindowShouldClose(m_glfwWindowHandler)) // todo: remove all this
    {
        currentFrameIndex = ++currentFrameIndex % MAX_FRAMES_IN_FLIGHT;
        auto &imageAvailableSemaphore = m_imageAvailableSemaphores[currentFrameIndex];
        auto &renderFinishedSemaphore = m_renderFinishedSemaphores[currentFrameIndex];

        vkWaitForFences(m_device.device, 1_u32t, &m_inFlightFramesFence[currentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(m_device.device, 1_u32t, &m_inFlightFramesFence[currentFrameIndex]);

        auto imageIndex = 0_u32t;
        vkAcquireNextImageKHR(m_device.device, m_swapchain.swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        VkPipelineStageFlags pipelineStagesFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1_u32t;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = pipelineStagesFlags;
        submitInfo.commandBufferCount = 1_u32t;
        submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1_u32t;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

        if (VK_SUCCESS != vkQueueSubmit(m_device.graphicQueue, 1_u32t, &submitInfo, m_inFlightFramesFence[currentFrameIndex]))
        {
            log << "vkQueueSubmit failed. Terminated." << std::endl;
            std::terminate();
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1_u32t;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1_u32t;
        presentInfo.pSwapchains = &m_swapchain.swapchain;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(m_device.graphicQueue, &presentInfo);       
        vkQueueWaitIdle(m_device.graphicQueue);
    }

    vkDeviceWaitIdle(m_device.device);
}
