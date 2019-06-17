#include "Renderer.hpp"

using namespace KompotEngine::Renderer;

Renderer::Renderer(GLFWwindow *window,
                   const std::string &windowName)
    : m_glfwWindowHandler(window),
      m_windowsName(windowName),
      m_isResized(false),
      m_device(windowName)
{
    Log::setupDebugCallback(m_device.getInstance(), m_device);
    createSurface();
    m_device.create(m_vkSurface);
    createCommandPool();
    createDescriptorSetLayout();
    m_resourcesMaker = new ResourcesMaker(m_device,
        m_vkCommandPool,
        m_vkDescriptorSetLayout);

    createSwapchain();
    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

void Renderer::recreateSwapchain()
{
    vkDeviceWaitIdle(m_device);

    cleanupSwapchain();

    createSwapchain();
    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
}

void Renderer::cleanupSwapchain()
{
    m_vkDepthImage.reset();
    for (auto framebuffer : m_vkFramebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    vkDestroyPipeline(m_device, m_vkPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_vkPipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_vkRenderPass, nullptr);
    m_vkSwapchainImages.clear();
    vkDestroySwapchainKHR(m_device, m_vkSwapchain, nullptr);
}

Renderer::~Renderer()
{
    delete m_resourcesMaker;
    m_world->clear();

    cleanupSwapchain();
    vkDestroyDescriptorSetLayout(m_device, m_vkDescriptorSetLayout, nullptr);

    for (auto i = 0_u64t; i < MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        vkDestroySemaphore(m_device, m_vkImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_vkRenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_vkInFlightFramesFence[i], nullptr);
    }
    vkDestroyCommandPool(m_device, m_vkCommandPool, nullptr);
    vkDestroySurfaceKHR(m_device.getInstance(), m_vkSurface, nullptr);

    Log::deleteDebugCallback();
}

void Renderer::run(const std::shared_ptr<KompotEngine::World>& world)
{
	m_world = world;

    auto currentFrameIndex = 0_u64t;
    while (!glfwWindowShouldClose(m_glfwWindowHandler))
    {
        currentFrameIndex = ++currentFrameIndex % MAX_FRAMES_IN_FLIGHT;
        auto &imageAvailableSemaphore = m_vkImageAvailableSemaphores[currentFrameIndex];
        auto &renderFinishedSemaphore = m_vkRenderFinishedSemaphores[currentFrameIndex];

        vkWaitForFences(m_device, 1_u32t, &m_vkInFlightFramesFence[currentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        m_world->loadObjects(*m_resourcesMaker);

        auto imageIndex = 0_u32t;
        auto resultCode = vkAcquireNextImageKHR(m_device,
                                                m_vkSwapchain,
                                                std::numeric_limits<uint64_t>::max(),
                                                imageAvailableSemaphore,
                                                nullptr,
                                                &imageIndex);

        if (resultCode == VK_ERROR_OUT_OF_DATE_KHR || m_isResized)
        {
            m_isResized = false;
            recreateSwapchain();
            continue;
        }

        // setup command buffer        

        VkRenderPassBeginInfo vkRenderPassBeginInfo = {};
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.renderPass = m_vkRenderPass;
        vkRenderPassBeginInfo.framebuffer = m_vkFramebuffers[imageIndex];
        vkRenderPassBeginInfo.renderArea.offset = {0_32t, 0_32t};
        vkRenderPassBeginInfo.renderArea.extent = m_vkExtent;

        std::array<VkClearValue, 2_u32t> vkClearValues;

        vkClearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        vkClearValues[1].depthStencil = {1.0f, 0};
        vkRenderPassBeginInfo.clearValueCount = vkClearValues.size();
        vkRenderPassBeginInfo.pClearValues = vkClearValues.data();

        //static auto lastTime = std::chrono::high_resolution_clock::now();
        //auto currentTime = std::chrono::high_resolution_clock::now();
        //float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        UnifromBufferObject mvpMatrix = {};
        mvpMatrix.view = glm::lookAt(glm::vec3(5.0f, 15.0f, 10.0f), glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        mvpMatrix.projection = glm::perspective(glm::radians(90.0f), static_cast<float>(m_vkExtent.width) / static_cast<float>(m_vkExtent.height), 0.01f, 50.0f);
        mvpMatrix.projection[1][1] *= -1;

        m_world->lock();

        for (auto&& [objectId, object] : m_world->getMeshObjects())
        {
            mvpMatrix.model = object->getModelMatrix();
            //mvpMatrix.model = glm::rotate(mvpMatrix.model, deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
            auto uniformMatricesBuffers = object->getUboBuffer();
            uniformMatricesBuffers->copyFromRawPointer(&mvpMatrix, sizeof(UnifromBufferObject));
        }

        auto commandBuffer = m_resourcesMaker->createSingleTimeCommandBuffer();

        VkDeviceSize vkOffsetsSizes[] = {0_u32t};
        vkCmdBeginRenderPass(commandBuffer, &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        for (auto&& [objectId, object]  : m_world->getMeshObjects())
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
            vkCmdBindVertexBuffers(commandBuffer, 0_u32t, 1_u32t, object->getMesh()->getVertexBuffer(), vkOffsetsSizes);
            vkCmdBindIndexBuffer(commandBuffer, *object->getMesh()->getIndecesBuffer(), 0_u32t, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0_u32t, 1_u32t,
                                    object->getDescriptorSet(), 0_u32t, nullptr);
            vkCmdDrawIndexed(commandBuffer, object->getMesh()->getIndicesCount(), 1_u32t, 0_u32t, 0_u32t, 0_u32t);
        }
        vkCmdEndRenderPass(commandBuffer);

        m_world->unlock();

        resultCode = vkEndCommandBuffer(commandBuffer);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::createCommandBuffers(): Function vkEndCommandBuffer call failed with a code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }

        // submit

        VkPipelineStageFlags pipelineStagesFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1_u32t;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = pipelineStagesFlags;
        submitInfo.commandBufferCount = 1_u32t;
        submitInfo.pCommandBuffers = commandBuffer;
        submitInfo.signalSemaphoreCount = 1_u32t;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

        resultCode = vkResetFences(m_device, 1_u32t, &m_vkInFlightFramesFence[currentFrameIndex]);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::run(): vkResetFences failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
        resultCode = vkQueueSubmit(m_device.getGraphicsQueue(), 1_u32t, &submitInfo, m_vkInFlightFramesFence[currentFrameIndex]);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::run(): vkQueueSubmit failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }        

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1_u32t;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1_u32t;
        presentInfo.pSwapchains = &m_vkSwapchain;
        presentInfo.pImageIndices = &imageIndex;

        resultCode = vkQueuePresentKHR(m_device.getGraphicsQueue(), &presentInfo);
        if (resultCode == VK_ERROR_OUT_OF_DATE_KHR ||
            resultCode == VK_SUBOPTIMAL_KHR ||
            m_isResized)
        {
            m_isResized = false;
            if(!glfwWindowShouldClose(m_glfwWindowHandler))
            {
                recreateSwapchain();
            }
        }
        else if(resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::run(): vkQueuePresentKHR failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
        resultCode = vkQueueWaitIdle(m_device.getGraphicsQueue());
        if(resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::run(): vkQueueWaitIdle failed with code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
        commandBuffer.free();
    }

    vkDeviceWaitIdle(m_device);
	m_world.reset();
}

void Renderer::resize()
{
    m_isResized = true;
}

void Renderer::createSurface()
{
    const auto resultCode = glfwCreateWindowSurface(m_device.getInstance(), m_glfwWindowHandler, nullptr, &m_vkSurface);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createSurface(): glfwCreateWindowSurface failed wih a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

SwapchainSupportDetails Renderer::getSwapchainDetails()
{
    SwapchainSupportDetails swapchainDetails {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device, m_vkSurface, &swapchainDetails.capabilities);

    auto formatsCount = 0_u32t;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_device, m_vkSurface, &formatsCount, nullptr);
    std::vector<VkSurfaceFormatKHR> vkSurfaceFormats(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_device, m_vkSurface, &formatsCount, vkSurfaceFormats.data());
    swapchainDetails.format = chooseSurfaceFormat(vkSurfaceFormats);

    auto presentModesCount = 0_u32t;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, m_vkSurface, &presentModesCount, nullptr);
    std::vector<VkPresentModeKHR> vkPresentModes(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, m_vkSurface, &presentModesCount, vkPresentModes.data());
    swapchainDetails.presentMode = choosePresentMode(vkPresentModes);

    return swapchainDetails;
}

VkSurfaceFormatKHR Renderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &vkSurfaceFormats)
{
    if (vkSurfaceFormats.size() == 1 && vkSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for(const auto &surfaceFormat: vkSurfaceFormats)
    {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surfaceFormat;
        }
    }

    return vkSurfaceFormats[0];
}

VkExtent2D Renderer::chooseExtent(const VkSurfaceCapabilitiesKHR &vkSurfaceCapabilities)
{
    glfwGetFramebufferSize(m_glfwWindowHandler, reinterpret_cast<int*>(&m_vkExtent.width), reinterpret_cast<int*>(&m_vkExtent.height));
    VkExtent2D vkExtent2D = m_vkExtent;
    vkExtent2D.width = std::clamp(vkExtent2D.width, vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.maxImageExtent.width);
    vkExtent2D.height = std::clamp(vkExtent2D.height, vkSurfaceCapabilities.minImageExtent.height, vkSurfaceCapabilities.maxImageExtent.height);
    return vkExtent2D;
}

VkPresentModeKHR Renderer::choosePresentMode(const std::vector<VkPresentModeKHR> &vkPresentModes)
{
    for (const auto &presentMode : vkPresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void Renderer::createSwapchain()
{
    const auto swapchainDetails = getSwapchainDetails();
    const auto queuefamilies = m_device.findQueueFamilies();
    uint32_t queuefamiliesIndicies[] = {
        queuefamilies.graphicFamilyIndex.value(),
        queuefamilies.presentFamilyIndex.value()
    };

    m_vkExtent = chooseExtent(swapchainDetails.capabilities);
    m_vkImageFormat = swapchainDetails.format.format;

    VkSwapchainCreateInfoKHR vkSwapchainCreateInfo = {};
    vkSwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfo.surface = m_vkSurface;
    vkSwapchainCreateInfo.minImageCount = swapchainDetails.capabilities.minImageCount + 1_u32t;
    vkSwapchainCreateInfo.imageFormat = swapchainDetails.format.format;
    vkSwapchainCreateInfo.imageColorSpace = swapchainDetails.format.colorSpace;
    vkSwapchainCreateInfo.imageExtent = m_vkExtent;
    vkSwapchainCreateInfo.imageArrayLayers = 1_u32t;
    vkSwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (queuefamilies.graphicFamilyIndex != queuefamilies.presentFamilyIndex)
    {
        vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        vkSwapchainCreateInfo.queueFamilyIndexCount = 2_u32t;
        vkSwapchainCreateInfo.pQueueFamilyIndices = queuefamiliesIndicies;
    }
    else
    {
        vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    vkSwapchainCreateInfo.preTransform = swapchainDetails.capabilities.currentTransform;
    vkSwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfo.presentMode = swapchainDetails.presentMode;
    vkSwapchainCreateInfo.clipped = VK_TRUE;
    vkSwapchainCreateInfo.oldSwapchain = nullptr;

    const auto resultCode = vkCreateSwapchainKHR(m_device, &vkSwapchainCreateInfo, nullptr, &m_vkSwapchain);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createSwapchain(): Function vkCreateSwapchainKHR call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
    auto swapchainImagesCount = 0_u32t;
    vkGetSwapchainImagesKHR(m_device, m_vkSwapchain, &swapchainImagesCount, nullptr);
    std::vector<VkImage> swapchainImages(swapchainImagesCount);
    vkGetSwapchainImagesKHR(m_device, m_vkSwapchain, &swapchainImagesCount, swapchainImages.data());

    for (auto& swapchainImage : swapchainImages)
    {
        auto image = m_resourcesMaker->createSwapchainImage(swapchainImage, m_vkImageFormat);
        m_vkSwapchainImages.push_back(image);
    }
    m_vkViewport.x = 0.0f;
    m_vkViewport.y = 0.0f;
    m_vkViewport.width = static_cast<float>(m_vkExtent.width);
    m_vkViewport.height = static_cast<float>(m_vkExtent.height);
    m_vkViewport.minDepth = 0.0f;
    m_vkViewport.maxDepth = 1.0f;

    m_vkRect.offset = {0_32t, 0_32t};
    m_vkRect.extent = m_vkExtent;
}

void Renderer::createRenderPass()
{
    VkAttachmentDescription vkColorAttachmentDescription = {};
    vkColorAttachmentDescription.format = m_vkImageFormat;
    vkColorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    vkColorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkColorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkColorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkColorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference vkColorAttachmentReference = {};
    vkColorAttachmentReference.attachment = 0_u32t;
    vkColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription vkDepthAttachmentDescripton = {};
    vkDepthAttachmentDescripton.format = VK_FORMAT_D32_SFLOAT;
    vkDepthAttachmentDescripton.samples = VK_SAMPLE_COUNT_1_BIT;
    vkDepthAttachmentDescripton.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkDepthAttachmentDescripton.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkDepthAttachmentDescripton.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkDepthAttachmentDescripton.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkDepthAttachmentDescripton.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkDepthAttachmentDescripton.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference vkDepthAttachmentReference = {};
    vkDepthAttachmentReference.attachment = 1_u32t;
    vkDepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription vkSubpassDescription = {};
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.colorAttachmentCount = 1_u32t;
    vkSubpassDescription.pColorAttachments = &vkColorAttachmentReference;
    vkSubpassDescription.pDepthStencilAttachment = &vkDepthAttachmentReference;

    std::array<VkAttachmentDescription, 2_u32t> vkAttachmentsDescriptions = {vkColorAttachmentDescription, vkDepthAttachmentDescripton};

    VkSubpassDependency vkSubpassDependency = {};
    vkSubpassDependency.srcSubpass = 0_u32t;
    vkSubpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    vkSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo vkRenderPassCreateInfo = {};
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.attachmentCount = vkAttachmentsDescriptions.size();
    vkRenderPassCreateInfo.pAttachments = vkAttachmentsDescriptions.data();
    vkRenderPassCreateInfo.subpassCount = 1_u32t;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 1_u32t;
    vkRenderPassCreateInfo.pDependencies = &vkSubpassDependency;

    const auto resultCode = vkCreateRenderPass(m_device, &vkRenderPassCreateInfo, nullptr, &m_vkRenderPass);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createRenderPass(): Function vkCreateRenderPass call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createFramebuffers()
{
    m_vkFramebuffers.resize(static_cast<uint32_t>(m_vkSwapchainImages.size()));
    for (auto i = 0_u64t; i < static_cast<uint64_t>(m_vkSwapchainImages.size()); ++i)
    {
        std::array<VkImageView, 2_u32t> vkImageViews= {m_vkSwapchainImages[i]->getImageView(), m_vkDepthImage->getImageView()};

        VkFramebufferCreateInfo vkFramebufferCreateInfo = {};
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.renderPass = m_vkRenderPass;
        vkFramebufferCreateInfo.attachmentCount = vkImageViews.size();
        vkFramebufferCreateInfo.pAttachments = vkImageViews.data();
        vkFramebufferCreateInfo.width = m_vkExtent.width;
        vkFramebufferCreateInfo.height = m_vkExtent.height;
        vkFramebufferCreateInfo.layers = 1_u32t;

        const auto resultCode = vkCreateFramebuffer(m_device, &vkFramebufferCreateInfo, nullptr, &m_vkFramebuffers[i]);
        if (resultCode != VK_SUCCESS)
        {
            Log::getInstance() << "Renderer::createFramebuffers(): Function vkCreateFramebuffer call failed with a code " << resultCode << ". Terminated." << std::endl;
            std::terminate();
        }
    }
}

void Renderer::createDescriptorSetLayout()
{
    m_vkDescriptorSetLayout = nullptr;

    VkDescriptorSetLayoutBinding vkUniformBufferDescriptorSetLayoutBinding = {};
    vkUniformBufferDescriptorSetLayoutBinding.binding = 0_u32t;
    vkUniformBufferDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkUniformBufferDescriptorSetLayoutBinding.descriptorCount = 1_u32t;
    vkUniformBufferDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding vkSamplerDescriptorSetlayourBinding = {};
    vkSamplerDescriptorSetlayourBinding.binding = 1_u32t;
    vkSamplerDescriptorSetlayourBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkSamplerDescriptorSetlayourBinding.descriptorCount = 1_u32t;
    vkSamplerDescriptorSetlayourBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2_u32t> vkDescriptorSetLayoutBindings = {vkUniformBufferDescriptorSetLayoutBinding, vkSamplerDescriptorSetlayourBinding};

    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {};
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.bindingCount = vkDescriptorSetLayoutBindings.size();
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBindings.data();

    const auto resultCode = vkCreateDescriptorSetLayout(m_device, &vkDescriptorSetLayoutCreateInfo, nullptr, &m_vkDescriptorSetLayout);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createDescriptorSetLayout(): Function vkCreateDescriptorSetLayout call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
};

void Renderer::createGraphicsPipeline()
{
    m_vkPipelineLayout = nullptr;
    Shader vertexShader("triangle.vert.spv", m_device);
    Shader fragmentShader("triangle.frag.spv", m_device);
    if (!vertexShader.load())
    {
        Log::getInstance() << "Renderer::createGraphicsPipeline(): Vertex shader loading error. Terminated." << std::endl;
        std::terminate();
    }
    if (!fragmentShader.load())
    {
        Log::getInstance() << "Renderer:createGraphicsPipeline(): Fragment shader loading error. Terminated." << std::endl;
        std::terminate();
    }

    VkPipelineShaderStageCreateInfo vkVertexShaderStageCreateInfo = {};
    vkVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkVertexShaderStageCreateInfo.module = vertexShader.get();
    vkVertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo vkFragmentShaderStageCreateInfo = {};
    vkFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkFragmentShaderStageCreateInfo.module = fragmentShader.get();
    vkFragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo vkPipelineShadersStagesCreateInfos[] = {vkVertexShaderStageCreateInfo, vkFragmentShaderStageCreateInfo};

    auto vertexBindingDescription = Vertex::getBindingDescription();
    auto vertexAttributesDescriptions = Vertex::getAttributeDescritptions();

    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStageCreateInfo = {};
    vkPipelineVertexInputStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStageCreateInfo.vertexBindingDescriptionCount = 1_u32t;
    vkPipelineVertexInputStageCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vkPipelineVertexInputStageCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributesDescriptions.size());
    vkPipelineVertexInputStageCreateInfo.pVertexAttributeDescriptions = vertexAttributesDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStageCreateInfo = {};
    vkPipelineInputAssemblyStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStageCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vkPipelineInputAssemblyStageCreateInfo.primitiveRestartEnable = VK_FALSE;

//    VkPipelineTessellationStateCreateInfo tessellationInfo = {};
//    tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
//    tessellationInfo

    VkPipelineViewportStateCreateInfo vkPipelineViewportStageCreateInfo = {};
    vkPipelineViewportStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStageCreateInfo.viewportCount = 1_u32t;
    vkPipelineViewportStageCreateInfo.pViewports = &m_vkViewport;
    vkPipelineViewportStageCreateInfo.scissorCount = 1_u32t;
    vkPipelineViewportStageCreateInfo.pScissors = &m_vkRect;

    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStageCreateInfo = {};
    vkPipelineRasterizationStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStageCreateInfo.depthClampEnable = VK_FALSE;
    vkPipelineRasterizationStageCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    vkPipelineRasterizationStageCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStageCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    vkPipelineRasterizationStageCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStageCreateInfo.depthBiasEnable = VK_FALSE;
    vkPipelineRasterizationStageCreateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStageCreateInfo = {};
    vkPipelineMultisampleStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStageCreateInfo.sampleShadingEnable = VK_FALSE;
    vkPipelineMultisampleStageCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStageCreateInfo = {};
    vkPipelineDepthStencilStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStageCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStageCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStageCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    vkPipelineDepthStencilStageCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilStageCreateInfo.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState = {};
    vkPipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
    vkPipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStageCreateInfo = {};
    vkPipelineColorBlendStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStageCreateInfo.logicOpEnable = VK_FALSE;
    vkPipelineColorBlendStageCreateInfo.attachmentCount = 1_u32t;
    vkPipelineColorBlendStageCreateInfo.pAttachments = &vkPipelineColorBlendAttachmentState;

    //VkPipelineDynamicStateCreateInfo dynamicInfo = {};

    VkPipelineLayoutCreateInfo VkPipelineLayoutCreateInfo = {};
    VkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkPipelineLayoutCreateInfo.setLayoutCount = 1_u32t;
    VkPipelineLayoutCreateInfo.pSetLayouts = &m_vkDescriptorSetLayout;

    auto resultCode = vkCreatePipelineLayout(m_device, &VkPipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createGraphicsPipeline(): Function vkCreatePipelineLayout call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }

    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo = {};
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.stageCount = 2_u32t;
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShadersStagesCreateInfos;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStageCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStageCreateInfo;
    vkGraphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = m_vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0_u32t;

    resultCode = vkCreateGraphicsPipelines(m_device, nullptr, 1_u32t, &vkGraphicsPipelineCreateInfo, nullptr, &m_vkPipeline);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createGraphicsPipeline(): Function vkCreateGraphicsPipelines call failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createCommandPool()
{
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo = {};
    const auto queueFamilies = m_device.findQueueFamilies();
    vkCommandPoolCreateInfo.sType  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.queueFamilyIndex = queueFamilies.graphicFamilyIndex.value();

    const auto resultCode = vkCreateCommandPool(m_device, &vkCommandPoolCreateInfo, nullptr, &m_vkCommandPool);
    if (resultCode != VK_SUCCESS)
    {
        Log::getInstance() << "Renderer::createCommandPool(): Function vkCreateCommandPool failed with a code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void Renderer::createSyncObjects()
{
    m_vkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkInFlightFramesFence.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo vkSemaphoreCreateInfo = {};
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo vkFenceCreateInfo = {};
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = 0_u64t; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (VK_SUCCESS != vkCreateSemaphore(m_device, &vkSemaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphores[i]) ||
            VK_SUCCESS != vkCreateSemaphore(m_device, &vkSemaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphores[i]) ||
            VK_SUCCESS != vkCreateFence(m_device, &vkFenceCreateInfo, nullptr, &m_vkInFlightFramesFence[i]))
        {
            Log::getInstance() << "Failed to create synchronization objects. Terminated." << std::endl;
            std::terminate();
        }
    }
}

void Renderer::createDepthResources()
{
    // VkFormat format = findDepthFormat(); in vulkan-tutorial.com guide
    m_vkDepthImage = m_resourcesMaker->createImage(
                          m_vkExtent,
                          1_u32t,
                          VK_FORMAT_D32_SFLOAT,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                          VK_IMAGE_ASPECT_DEPTH_BIT);
    m_vkDepthImage->transitImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_resourcesMaker->createSingleTimeCommandBuffer());
}
