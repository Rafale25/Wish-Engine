#include "Context.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <print>

static inline void chk(VkResult result) {
	if (result != VK_SUCCESS) {
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}

inline void Context::chkSwapchain(VkResult result) {
	if (result < VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_updateSwapchain = true;
			return;
		}
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}

void Context::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    ctx->m_framebufferWidth = width;
    ctx->m_framebufferHeight = height;
}


struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

/*

void Context::makeModelStuff() {

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 uv;
    };

    const VkDeviceSize indexCount{6};
    std::vector<Vertex> vertices{
        {{-1,  1, 0}, {1, 0, 0}, {0, 0}},
        {{ 1,  1, 0}, {0, 1, 0}, {1, 0}},
        {{-1, -1, 0}, {0, 0, 1}, {0, 1}},
        {{ 1, -1, 0}, {0, 1, 1}, {1, 1}},
    };
    std::vector<uint16_t> indices{
        0, 2, 1,
        2, 1, 3,
    };

    VkDeviceSize vBufSize{ sizeof(Vertex) * vertices.size() };
    VkDeviceSize iBufSize{ sizeof(uint16_t) * indices.size() };
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vBufSize + iBufSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };


    VkBuffer vBuffer{ VK_NULL_HANDLE };
    VmaAllocation vBufferAllocation{ VK_NULL_HANDLE };
    VmaAllocationCreateInfo vBufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo vBufferAllocInfo{};
    chk(vmaCreateBuffer(m_allocator, &bufferCI, &vBufferAllocCI, &vBuffer, &vBufferAllocation, &vBufferAllocInfo));

    memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
    memcpy(((char*)vBufferAllocInfo.pMappedData) + vBufSize, indices.data(), iBufSize);
}
*/

void Context::initWindow() {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(640, 480, applicationName.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // glfwMakeContextCurrent(window);

    if (!glfwVulkanSupported()) {
        std::cerr << "GLFW doesn't support Vulkan?!!\n";
		exit(-1);
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // User pointer to Context
    glfwSetWindowUserPointer(window, this);

    glfwGetFramebufferSize(window, &m_framebufferWidth, &m_framebufferHeight);
}

// void loadShader(const char* name, const char* path) {
//     Slang::ComPtr<slang::IModule> slangModule{
//         slangSession->loadModuleFromSource(name, path, nullptr, nullptr)
//     };
//     Slang::ComPtr<ISlangBlob> spirv;
//     slangModule->getTargetCode(0, spirv.writeRef());
// }

void Context::updateSwapchain() {
    int deviceIndex = 0; // TEMPPPPPPP
    m_updateSwapchain = false;
    vkDeviceWaitIdle(m_device);
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_devices[deviceIndex], m_surface, &m_surfaceCaps));
    m_swapchainCI.oldSwapchain = m_swapchain;
    m_swapchainCI.imageExtent = { .width = static_cast<uint32_t>(m_framebufferWidth), .height = static_cast<uint32_t>(m_framebufferHeight) };
    chk(vkCreateSwapchainKHR(m_device, &m_swapchainCI, nullptr, &m_swapchain));
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
    }
    chk(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, nullptr));
    m_swapchainImages.resize(m_swapchainImageCount);
    chk(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, m_swapchainImages.data()));
    m_swapchainImageViews.resize(m_swapchainImageCount);
    for (auto i = 0; i < m_swapchainImageCount; i++) {
        VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = SWAPCHAIN_IMAGE_FORMAT,
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        };
        chk(vkCreateImageView(m_device, &viewCI, nullptr, &m_swapchainImageViews[i]));
    }
    vkDestroySwapchainKHR(m_device, m_swapchainCI.oldSwapchain, nullptr);
    vmaDestroyImage(m_allocator, m_depthImage, m_depthImageAllocation);
    vkDestroyImageView(m_device, m_depthImageView, nullptr);

    m_depthImageCI.extent = { .width = static_cast<uint32_t>(m_framebufferWidth), .height = static_cast<uint32_t>(m_framebufferHeight), .depth = 1 };
    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    chk(vmaCreateImage(m_allocator, &m_depthImageCI, &allocCI, &m_depthImage, &m_depthImageAllocation, nullptr));
    VkImageViewCreateInfo viewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_depthFormat,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
    };
    chk(vkCreateImageView(m_device, &viewCI, nullptr, &m_depthImageView));
}

void Context::init() {
    initWindow();

    // VK_FORMAT_D32_SFLOAT_S8_UINT

    // MARK: Instance
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = applicationName.c_str(),
        .apiVersion = VK_API_VERSION_1_4
    };

    uint32_t instanceExtensionsCount{ 0 };
    char const* const* instanceExtensions{ glfwGetRequiredInstanceExtensions(&instanceExtensionsCount) };

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = instanceExtensionsCount,
        .ppEnabledExtensionNames = instanceExtensions,
    };

    chk(vkCreateInstance(&instanceCI, nullptr, &m_instance));

    // MARK: Device count

    chk(vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, nullptr));
    // std::vector<VkPhysicalDevice> devices(m_deviceCount);
    m_devices.resize(m_deviceCount);
    chk(vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, m_devices.data()));

    std::print("deviceCount : {}", m_deviceCount);


    // MARK: Choose device

    VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(m_devices[m_deviceIndex], &deviceProperties);
    std::cout << "Selected device: " << deviceProperties.properties.deviceName <<  "\n";

    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(m_devices[m_deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_devices[m_deviceIndex], &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamily{ 0 };
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamily = i;
            break;
        }
    }

    if (glfwGetPhysicalDevicePresentationSupport(m_instance, m_devices[m_deviceIndex], queueFamily) == GLFW_FALSE) {
		std::cerr << "Graphic queue doesn't support presentation!\n";
		exit(-1);
    }


    // MARK: Device, Queue

    constexpr float qfpriorities{ 1.0f };
    VkDeviceQueueCreateInfo queueCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    const std::vector<const char*> deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME,
    };

    const VkPhysicalDeviceFeatures enabledVk10Features{
        .samplerAnisotropy = VK_TRUE
    };
    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledVk12Features,
        .synchronization2 = true,
        .dynamicRendering = true,
    };
    const VkPhysicalDeviceVulkan14Features enabledVk14Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = &enabledVk13Features,
        .maintenance5 = VK_TRUE,
    };

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk14Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features
    };

    chk(vkCreateDevice(m_devices[m_deviceIndex], &deviceCI, nullptr, &m_device));
    vkGetDeviceQueue(m_device, queueFamily, 0, &m_queue);


    // MARK: VMA Allocator
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage
    };
    VmaAllocatorCreateInfo allocatorCI{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = m_devices[m_deviceIndex],
        .device = m_device,
        .pVulkanFunctions = &vkFunctions,
        .instance = m_instance
    };

    chk(vmaCreateAllocator(&allocatorCI, &m_allocator));


    // MARK: Window, Surface
    chk(glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface));

    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_devices[m_deviceIndex], m_surface, &m_surfaceCaps));

    VkExtent2D swapchainExtent{ m_surfaceCaps.currentExtent };
    if (m_surfaceCaps.currentExtent.width == 0xFFFFFFFF) {
		swapchainExtent = {
            .width = static_cast<uint32_t>(m_framebufferWidth),
            .height = static_cast<uint32_t>(m_framebufferHeight)
        };
	}

    // MARK: Swapchain
    m_swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
        .minImageCount = m_surfaceCaps.minImageCount,
        .imageFormat = SWAPCHAIN_IMAGE_FORMAT,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{ .width = swapchainExtent.width, .height = swapchainExtent.height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    chk(vkCreateSwapchainKHR(m_device, &m_swapchainCI, nullptr, &m_swapchain));

    chk(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, nullptr));
    m_swapchainImages.resize(m_swapchainImageCount);
    chk(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, m_swapchainImages.data()));
    m_swapchainImageViews.resize(m_swapchainImageCount);

	for (uint32_t i = 0; i < m_swapchainImageCount; ++i) {
		VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = SWAPCHAIN_IMAGE_FORMAT,
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
        };
		chk(vkCreateImageView(m_device, &viewCI, nullptr, &m_swapchainImageViews[i]));
	}

    // VkFormat depthFormat{ VK_FORMAT_D32_SFLOAT_S8_UINT };
    std::vector<VkFormat> depthFormatList{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    // VkFormat m_depthFormat{ VK_FORMAT_UNDEFINED };
    for (VkFormat& format : depthFormatList) {
        VkFormatProperties2 formatProperties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        vkGetPhysicalDeviceFormatProperties2(m_devices[m_deviceIndex], format, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            m_depthFormat = format;
            break;
        }
    }

    m_depthImageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_depthFormat,
        // .extent{.width = window.getSize().x, .height = window.getSize().y, .depth = 1 },
        .extent{.width = (uint32_t)m_framebufferWidth, .height = (uint32_t)m_framebufferHeight, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    chk(vmaCreateImage(m_allocator, &m_depthImageCI, &allocCI, &m_depthImage, &m_depthImageAllocation, nullptr));

    m_depthViewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_depthFormat,
        .subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
    };
    chk(vkCreateImageView(m_device, &m_depthViewCI, nullptr, &m_depthImageView));


    // MARK: ShaderData
    for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
        VkBufferCreateInfo uBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(ShaderData),
            .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        VmaAllocationCreateInfo uBufferAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        chk(vmaCreateBuffer(m_allocator, &uBufferCI, &uBufferAllocCI, &m_shaderDataBuffers[i].buffer, &m_shaderDataBuffers[i].allocation, &m_shaderDataBuffers[i].allocationInfo));

        VkBufferDeviceAddressInfo uBufferBdaInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = m_shaderDataBuffers[i].buffer
        };
        m_shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(m_device, &uBufferBdaInfo);
    }

    // MARK: Semaphore

    VkSemaphoreCreateInfo semaphoreCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (uint32_t i = 0; i < maxFramesInFlight; i++) {
        chk(vkCreateFence(m_device, &fenceCI, nullptr, &m_fences[i]));
        chk(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &m_presentSemaphores[i]));
    }
    m_renderSemaphores.resize(m_swapchainImages.size());
    for (auto& semaphore : m_renderSemaphores) {
        chk(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
    }

    // MARK: Command pool
    VkCommandPoolCreateInfo commandPoolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily
    };
    chk(vkCreateCommandPool(m_device, &commandPoolCI, nullptr, &m_commandPool));


    // MARK: Command buffer
    VkCommandBufferAllocateInfo cbAllocCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_commandPool,
        .commandBufferCount = maxFramesInFlight
    };
    chk(vkAllocateCommandBuffers(m_device, &cbAllocCI, m_commandBuffers.data()));

    // MARK: Slang
    slang::createGlobalSession(m_slangGlobalSession.writeRef());

    auto slangTargets{ std::to_array<slang::TargetDesc>({ {
        .format{SLANG_SPIRV},
        .profile{m_slangGlobalSession->findProfile("spirv_1_4")}
    } })};
    auto slangOptions{ std::to_array<slang::CompilerOptionEntry>({ {
        slang::CompilerOptionName::EmitSpirvDirectly,
        {slang::CompilerOptionValueKind::Int, 1}
    } })};
    slang::SessionDesc slangSessionDesc{
        .targets{slangTargets.data()},
        .targetCount{SlangInt(slangTargets.size())},
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .compilerOptionEntries{slangOptions.data()},
        .compilerOptionEntryCount{uint32_t(slangOptions.size())}
    };
    m_slangGlobalSession->createSession(slangSessionDesc, m_slangSession.writeRef());

}

void Context::beginRendering() {

}

void Context::endRendering() {

}
