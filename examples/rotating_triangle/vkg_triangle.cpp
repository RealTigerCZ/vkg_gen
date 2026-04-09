// Test example using the generated vk:: wrapper (out.hpp / out.cpp)
// Read README.md for more info

#include "out.hpp"

// Provide C Vulkan types so GLFW exposes its Vulkan surface creation functions
typedef vk::Instance::HandleType VkInstance;
typedef vk::SurfaceKHR::HandleType VkSurfaceKHR;
typedef vk::PhysicalDevice::HandleType VkPhysicalDevice;
typedef int32_t VkResult;
typedef void VkAllocationCallbacks;
using PFN_vkGetInstanceProcAddr = vk::PFN_vkGetInstanceProcAddr;
#define VK_NULL_HANDLE nullptr
#define VK_SUCCESS 0
#define VK_VERSION_1_0 1

#include <GLFW/glfw3.h>

#define RESIZABLE 1

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <thread>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;
const int TARGET_FPS = 60;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    "VK_KHR_swapchain"
};


struct PushConstants {
    float time;
    float aspectRatio;
};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    static std::vector<uint32_t> readSpv(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filename);
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
        file.seekg(0);
        file.read((char*)buffer.data(), fileSize);
        file.close();
        return buffer;
    }

private:
    GLFWwindow* window;

    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice = nullptr;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::SwapchainKHR swapchain;
    vk::vector<vk::Image> swapchainImages;
    vk::Format swapchainImageFormat;
    vk::Extent2D swapchainExtent;
    std::vector<vk::ImageView> swapchainImageViews;
    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;
    std::vector<vk::Framebuffer> swapchainFramebuffers;
    vk::CommandPool commandPool;

    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    uint32_t currentFrame = 0;

    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

#ifdef RESIZABLE
    bool framebufferResized = false;
#endif

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#ifdef RESIZABLE
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "VKG New Triangle (generated wrapper)", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
#else
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "VKG New Triangle", nullptr, nullptr);
#endif
    }

    void initVulkan() {
        vk::loadLib();

        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffer();
        createSyncObjects();

        startTime = std::chrono::high_resolution_clock::now();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            glfwPollEvents();
            drawFrame();
            auto frameEnd = std::chrono::high_resolution_clock::now();

            auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();
            const long targetDuration = 1000 / TARGET_FPS;
            if (frameDuration < targetDuration) {
                std::this_thread::sleep_for(std::chrono::milliseconds(targetDuration - frameDuration));
            }
        }
        vk::deviceWaitIdle();
    }

    void cleanup() {
        cleanupSwapChain();
        vk::destroy(graphicsPipeline);
        vk::destroy(pipelineLayout);
        vk::destroy(renderPass);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::destroy(renderFinishedSemaphores[i]);
            vk::destroy(imageAvailableSemaphores[i]);
            vk::destroy(inFlightFences[i]);
        }
        vk::destroy(commandPool);
        vk::destroy(surface);
        vk::cleanUp();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void createInstance() {
        vk::ApplicationInfo appInfo{};
        appInfo.sType = vk::StructureType::eApplicationInfo;
        appInfo.pApplicationName = "VKG New Triangle";
        appInfo.apiVersion = vk::ApiVersion10;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        vk::InstanceCreateInfo createInfo{};
        createInfo.sType = vk::StructureType::eInstanceCreateInfo;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = 0;

        vk::initInstance(createInfo);
    }

    void createSurface() {
        vk::SurfaceKHR::HandleType rawSurface;
        if (glfwCreateWindowSurface(vk::instance().handle(), window, nullptr, &rawSurface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
        surface = rawSurface;
    }

    void pickPhysicalDevice() {
        auto devices = vk::enumeratePhysicalDevices();

        for (const auto& dev : devices) {
            auto props = vk::getPhysicalDeviceProperties(dev);
            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                physicalDevice = dev;
                std::cout << "Selected GPU: " << props.deviceName << std::endl;
                return;
            }
        }
        physicalDevice = devices[0];
    }

    void createLogicalDevice() {
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
        queueCreateInfo.queueFamilyIndex = 0;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        vk::DeviceCreateInfo createInfo{};
        createInfo.sType = vk::StructureType::eDeviceCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        vk::initDevice(physicalDevice, createInfo);

        graphicsQueue = vk::getDeviceQueue(0, 0);
        presentQueue = vk::getDeviceQueue(0, 0);
    }

    void createSwapChain() {
        auto capabilities = vk::getPhysicalDeviceSurfaceCapabilitiesKHR(surface);
        auto formats = vk::getPhysicalDeviceSurfaceFormatsKHR(surface);
        auto presentModes = vk::getPhysicalDeviceSurfacePresentModesKHR(surface);

        vk::Extent2D extent;
        if (capabilities.currentExtent.width != UINT32_MAX) {
            extent = capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
        for (const auto& availablePresentMode : presentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                presentMode = vk::PresentModeKHR::eMailbox;
                break;
            }
            if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
                presentMode = vk::PresentModeKHR::eImmediate;
            }
        }

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = vk::Format::eB8G8R8A8Srgb;
        createInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = 1;

        swapchain = vk::createSwapchainKHR(createInfo);
        swapchainImages = vk::getSwapchainImagesKHR(swapchain);

        swapchainImageFormat = vk::Format::eB8G8R8A8Srgb;
        swapchainExtent = extent;
    }

    void createImageViews() {
        swapchainImageViews.resize(swapchainImages.size());
        for (size_t i = 0; i < swapchainImages.size(); i++) {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.sType = vk::StructureType::eImageViewCreateInfo;
            createInfo.image = swapchainImages[i];
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.format = swapchainImageFormat;
            createInfo.components = { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity };
            createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            swapchainImageViews[i] = vk::createImageView(createInfo);
        }
    }

    void createRenderPass() {
        vk::AttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainImageFormat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        renderPass = vk::createRenderPass(renderPassInfo);
    }

    vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code) {
        vk::ShaderModuleCreateInfo createInfo{};
        createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        createInfo.codeSize = code.size() * 4;
        createInfo.pCode = code.data();
        return vk::createShaderModule(createInfo);
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = readSpv("vert_rotating.spv");
        auto fragShaderCode = readSpv("frag.spv");
        vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        rasterizer.depthClampEnable = 0;
        rasterizer.rasterizerDiscardEnable = 0;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eClockwise;

        vk::PipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = 0;

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        vk::PushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        pipelineLayout = vk::createPipelineLayout(pipelineLayoutInfo);

        vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;

        graphicsPipeline = vk::createGraphicsPipelines(nullptr, 1, pipelineInfo);

        vk::destroy(fragShaderModule);
        vk::destroy(vertShaderModule);
    }

    void createFramebuffers() {
        swapchainFramebuffers.resize(swapchainImageViews.size());
        for (size_t i = 0; i < swapchainImageViews.size(); i++) {
            vk::ImageView attachments[] = { swapchainImageViews[i] };
            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapchainExtent.width;
            framebufferInfo.height = swapchainExtent.height;
            framebufferInfo.layers = 1;
            swapchainFramebuffers[i] = vk::createFramebuffer(framebufferInfo);
        }
    }

    void createCommandPool() {
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        poolInfo.queueFamilyIndex = 0;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        commandPool = vk::createCommandPool(poolInfo);
    }

    void createCommandBuffer() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
            allocInfo.commandPool = commandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = 1;
            commandBuffers[i] = vk::allocateCommandBuffers(allocInfo);
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        vk::SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = vk::createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = vk::createSemaphore(semaphoreInfo);
            inFlightFences[i] = vk::createFence(fenceInfo);
        }
    }

    void drawFrame() {
        vk::waitForFences(1, &inFlightFences[currentFrame], 1, UINT64_MAX);

        uint32_t imageIndex;
        vk::Result result = vk::acquireNextImageKHR_noThrow(swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapChain();
            return;
        } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire image!");
        }

        vk::resetFences(1, &inFlightFences[currentFrame]);
        vk::resetCommandBuffer(commandBuffers[currentFrame], vk::CommandBufferResetFlags{});

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
        vk::beginCommandBuffer(commandBuffers[currentFrame], beginInfo);

        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassBeginInfo;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchainExtent;
        vk::ClearValue clearColor{};
        clearColor.color.float32[0] = 0.0f;
        clearColor.color.float32[1] = 0.0f;
        clearColor.color.float32[2] = 0.0f;
        clearColor.color.float32[3] = 1.0f;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vk::cmdBeginRenderPass(commandBuffers[currentFrame], renderPassInfo, vk::SubpassContents::eInline);
        vk::cmdBindPipeline(commandBuffers[currentFrame], vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        // Update Rotation & Aspect Ratio
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        PushConstants push{};
        push.time = time;
        push.aspectRatio = (float)swapchainExtent.width / (float)swapchainExtent.height;

        vk::cmdPushConstants(commandBuffers[currentFrame], pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), &push);

        vk::vector<vk::Viewport> viewports(1);
        viewports[0] = { .width = (float)swapchainExtent.width, .height = (float)swapchainExtent.height, .maxDepth = 1.0f };

        vk::cmdSetViewport(commandBuffers[currentFrame], 0, viewports);

        vk::vector<vk::Rect2D> scissors(1);
        scissors[0] = { .extent = swapchainExtent };
        vk::cmdSetScissor(commandBuffers[currentFrame], 0, scissors);

        vk::cmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
        vk::cmdEndRenderPass(commandBuffers[currentFrame]);
        vk::endCommandBuffer(commandBuffers[currentFrame]);

        // Submit
        vk::SubmitInfo submitInfo{};
        submitInfo.sType = vk::StructureType::eSubmitInfo;
        vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vk::vector<vk::SubmitInfo> submitInfos(1);
        submitInfos[0] = submitInfo;

        vk::queueSubmit(graphicsQueue, submitInfos, inFlightFences[currentFrame]);

        // Present
        vk::PresentInfoKHR presentInfo{};
        presentInfo.sType = vk::StructureType::ePresentInfoKHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        vk::SwapchainKHR swapChains[] = { swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vk::queuePresentKHR_noThrow(presentQueue, presentInfo);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void cleanupSwapChain() {
        for (auto framebuffer : swapchainFramebuffers) vk::destroy(framebuffer);
        for (auto imageView : swapchainImageViews) vk::destroy(imageView);
        vk::destroy(swapchain);
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
        vk::deviceWaitIdle();
        cleanupSwapChain();
        createSwapChain();
        createImageViews();
        createFramebuffers();
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
};

int main() {
    HelloTriangleApplication app;
    try { app.run(); }
    catch (const std::exception& e) { std::cerr << e.what() << std::endl; return EXIT_FAILURE; }
    return EXIT_SUCCESS;
}
