#include "Context.hpp"
#include "Logger.hpp"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

static inline void chk(VkResult result) {
	if (result != VK_SUCCESS) {
        logE("Vulkan call returned an error: {}", static_cast<int32_t>(result));
		exit(result);
	}
}

inline void Context::chkSwapchain(VkResult result) {
	if (result < VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_updateSwapchain = true;
			return;
		}
        logE("Vulkan call returned an error: {}", static_cast<int32_t>(result));
		exit(result);
	}
}

Context::~Context() {
    cleanup();
    glfwTerminate();
}

void Context::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    if (key >= 0 && key < GLFW_KEY_LAST) {
        ctx->m_keystate[key] = action > 0 ? 1 : 0;
    }

    if (action == GLFW_PRESS)
        ctx->m_currentView->onKeyPress(key);
    else if (action == GLFW_RELEASE)
        ctx->m_currentView->onKeyRelease(key);
}

void Context::cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    ctx->m_mouseDeltaX = x - ctx->m_mouseX;
    ctx->m_mouseDeltaY = y - ctx->m_mouseY;

    ctx->m_currentView->onMouseMotion(x, y, ctx->m_mouseDeltaX, ctx->m_mouseDeltaY);

    ctx->m_mouseX = x;
    ctx->m_mouseY = y;

    // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    //     ctx->m_currentView->onMouseDrag(x, y, ctx->m_mouseDeltaX, ctx->m_mouseDeltaY);
    // }
}

// void Context::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
// {
//     Context* ctx = (Context*)glfwGetWindowUserPointer(window);

//     if (action == GLFW_PRESS)
//         ctx->m_currentView->onMousePress(ctx->m_mouseX, ctx->m_mouseY, button);
//     else if (action == GLFW_RELEASE)
//         ctx->m_currentView->onMouseRelease(ctx->m_mouseX, ctx->m_mouseY, button);
// }

void Context::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    ctx->m_updateSwapchain = true;

    ctx->m_framebufferWidth = width;
    ctx->m_framebufferHeight = height;
    ctx->m_currentView->onResize(width, height);
}

void Context::setView(View& view) {
    m_currentView->onExitView();
    m_currentView = &view;
    view.onEnterView();

    // call resize callback on first frame
    glfwGetFramebufferSize(window, &m_framebufferWidth, &m_framebufferHeight);
    framebufferSizeCallback(window, m_framebufferWidth, m_framebufferHeight);
}

bool Context::isKeyDown(int32_t key) const {
    return m_keystate[key] == GLFW_PRESS;
}

void Context::setCursorEnabled(bool enabled) {
    glfwSetInputMode(window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

    // Fix mouse pos jumping when disabling it causing big delta between previous and new pos
    if (!enabled) {
        double x{}, y{};
        glfwGetCursorPos(window, &x, &y);
        m_mouseX = x;
        m_mouseY = y;
    }
}

bool Context::isCursorEnabled() const {
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

void Context::toggleCursor() {
    setCursorEnabled(!isCursorEnabled());
}


// bool Context::isKeyPressed() const {
//     // TODO: implement that
//     return false;
// }

// bool Context::isKeyReleased() const {
// }


void Context::run() {
    double startTime = glfwGetTime();
    double lastFrameTime = startTime;

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        double time = glfwGetTime();
        m_timeSinceStart = time - startTime;
        double deltaTime = time - lastFrameTime;
        lastFrameTime = time;

        m_currentView->onUpdate(m_timeSinceStart, deltaTime);

        beginRendering();
        m_currentView->onDraw(m_timeSinceStart, deltaTime);
        endRendering();

        glfwPollEvents();
    }

    m_currentView->onExitView();
}

void Context::initWindow() {
    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(640, 480, applicationName.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        return;
    }

    // glfwMakeContextCurrent(window);

    if (!glfwVulkanSupported()) {
        logE("GLFW doesn't support Vulkan?!!");
		exit(-1);
    }

    glfwSetKeyCallback(window, keyCallback);
    // glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // User pointer to Context
    glfwSetWindowUserPointer(window, this);

    glfwGetFramebufferSize(window, &m_framebufferWidth, &m_framebufferHeight);
}

void Context::cleanup() {
    chk(vkDeviceWaitIdle(m_device));

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);

    for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
        vkDestroyFence(m_device, m_fences[i], nullptr);
        vkDestroySemaphore(m_device, m_presentSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_renderSemaphores[i], nullptr);
    }

    vkDestroyFence(m_device, m_oneTimeFence, nullptr);

    for (uint32_t i = 0 ; i < maxFramesInFlight; ++i) {
        vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
    }
    m_depthTexture.destroy();

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);

    if (ENABLE_VALIDATION_LAYERS) {
        destroyDebugUtilsMessenger();
    }

    vkDestroyInstance(m_instance, nullptr);
}

void Context::updateSwapchain() {
    m_updateSwapchain = false;
    vkDeviceWaitIdle(m_device);
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_devices[m_deviceIndex], m_surface, &m_surfaceCaps));
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
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
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

    m_depthTexture.destroy();

    m_depthTexture.create(
        m_depthFormat,
        m_framebufferWidth,
        m_framebufferHeight,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL logCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    (void)messageType;
    (void)pUserData;

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: logT("{}", pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    logI("{}", pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: logW("{}", pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   logE("{}", pCallbackData->pMessage); break;
        default:                                              logM("{}", pCallbackData->pMessage); break;
    }

    return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity =
                            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = logCallback,
        .pUserData = NULL
    };

    return createInfo;
}

void Context::createDebugUtilsMessenger() {
    VkDebugUtilsMessengerEXT logger{ nullptr };
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    VkDebugUtilsMessengerCreateInfoEXT createInfo = getDebugUtilsMessengerCreateInfo();

    if (func) {
        func(m_instance, &createInfo, NULL, &logger);
    }
    m_logger = logger;
}

void Context::destroyDebugUtilsMessenger() {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func && m_logger != NULL) {
        func(m_instance, m_logger, NULL);
    }
}

static std::vector<const char*> getExtensions() {
    uint32_t instanceExtensionsCount{ 0 };
    const char* const* instanceExtensions{ glfwGetRequiredInstanceExtensions(&instanceExtensionsCount) };

    std::vector<const char*> instanceExtensionsVector(instanceExtensions, instanceExtensions + instanceExtensionsCount);

    if (Context::ENABLE_VALIDATION_LAYERS) {
        instanceExtensionsVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef __APPLE__
    instanceExtensionsVector.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    // Required by VK_KHR_portability_subset at device-creation time; many loaders
    // also want it at instance level. Safe to always include on Apple.
    instanceExtensionsVector.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    return instanceExtensionsVector;
}

void Context::init() {
    initWindow();

    // MARK: Instance
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = applicationName.c_str(),
        .apiVersion = VK_API_VERSION_1_4
    };

    auto instanceExtensions = getExtensions();

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#ifdef __APPLE__
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };


    if (ENABLE_VALIDATION_LAYERS) {
        logI("Validation Layers enabled");
        VkDebugUtilsMessengerCreateInfoEXT logCallback = getDebugUtilsMessengerCreateInfo();

        instanceCI.pNext = &logCallback;
        instanceCI.enabledLayerCount = validationLayers.size();
        instanceCI.ppEnabledLayerNames = validationLayers.data();
    } else {
        instanceCI.pNext = nullptr;
        instanceCI.enabledLayerCount = 0;
        instanceCI.ppEnabledLayerNames = nullptr;
    }

    chk(vkCreateInstance(&instanceCI, nullptr, &m_instance));

    if (ENABLE_VALIDATION_LAYERS) {
        createDebugUtilsMessenger();
    }

    // MARK: Device count

    chk(vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, nullptr));
    m_devices.resize(m_deviceCount);
    chk(vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, m_devices.data()));
    logI("deviceCount: {}", m_deviceCount);

    // MARK: Choose device

    VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(m_devices[m_deviceIndex], &deviceProperties);
    logI("Selected device: {}", deviceProperties.properties.deviceName);

    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(m_devices[m_deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_devices[m_deviceIndex], &queueFamilyCount, queueFamilies.data());
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_queueFamily = i;
            break;
        }
    }

    if (glfwGetPhysicalDevicePresentationSupport(m_instance, m_devices[m_deviceIndex], m_queueFamily) == GLFW_FALSE) {
        logE("Graphic queue doesn't support presentation.");
		exit(-1);
    }


    // MARK: Device, Queue

    constexpr float qfpriorities{ 1.0f };
    VkDeviceQueueCreateInfo queueCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    const std::vector<const char*> deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
        "VK_KHR_portability_subset" // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#endif
        // VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME, // not supported by MoltenVK
    };

    const VkPhysicalDeviceFeatures enabledVk10Features{
        .fillModeNonSolid = VK_TRUE,
        // .wideLines = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };
    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true,
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
    vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);


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

    uint32_t requestedImageCount = std::max(m_surfaceCaps.minImageCount, 2u);
    if (m_surfaceCaps.maxImageCount > 0) {
        requestedImageCount = std::min(requestedImageCount, m_surfaceCaps.maxImageCount);
    }

    // MARK: Swapchain
    m_swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
        .minImageCount = requestedImageCount, //m_surfaceCaps.minImageCount,
        .imageFormat = SWAPCHAIN_IMAGE_FORMAT,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
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

    // MARK: Depth Image

    std::vector<VkFormat> depthFormatList{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    for (VkFormat& format : depthFormatList) {
        VkFormatProperties2 formatProperties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        vkGetPhysicalDeviceFormatProperties2(m_devices[m_deviceIndex], format, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            m_depthFormat = format;
            break;
        }
    }

    m_depthTexture.create(
        m_depthFormat,
        m_framebufferWidth,
        m_framebufferHeight,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT);

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
        .queueFamilyIndex = m_queueFamily
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
        .format = SLANG_SPIRV,
        .profile = m_slangGlobalSession->findProfile("spirv_1_4")
    } })};
    auto slangOptions{ std::to_array<slang::CompilerOptionEntry>({ {
        slang::CompilerOptionName::EmitSpirvDirectly,
        {slang::CompilerOptionValueKind::Int, 1}
    } })};

    // TODO: should probably not be hardcoded
    const std::vector<const char*> searchPaths = { "./src/shaders" };
    slang::SessionDesc slangSessionDesc{
        .targets = slangTargets.data(),
        .targetCount = SlangInt(slangTargets.size()),
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .searchPaths = searchPaths.data(),
        .searchPathCount = static_cast<SlangInt>(searchPaths.size()),
        .compilerOptionEntries = slangOptions.data(),
        .compilerOptionEntryCount = static_cast<uint32_t>(slangOptions.size()),
    };
    m_slangGlobalSession->createSession(slangSessionDesc, m_slangSession.writeRef());

    initOneTimeCommand();

    initImgui();
}

void Context::initImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // VkDescriptorPool imguiDescriptorPool;
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only


    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE }
    };
    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
        .poolSizeCount = 1,
        .pPoolSizes = poolSizes
    };
    vkCreateDescriptorPool(m_device, &poolCI, nullptr, &m_imguiDescriptorPool);

    VkPipelineRenderingCreateInfo pipelineRenderingCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &SWAPCHAIN_IMAGE_FORMAT
    };

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = API_VERSION;
    init_info.Instance = m_instance;
    init_info.PhysicalDevice = m_devices[m_deviceIndex];
    init_info.Device = m_device;
    init_info.Queue = m_queue;
    init_info.QueueFamily = m_queueFamily;
    init_info.DescriptorPool = m_imguiDescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = m_swapchainImageCount;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingCI;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.UseDynamicRendering = true;
    ImGui_ImplVulkan_Init(&init_info);
}

void Context::beginRendering() {
    if (m_updateSwapchain) {
        updateSwapchain();
    }
    chk(vkWaitForFences(m_device, 1, &m_fences[m_frameIndex], true, UINT64_MAX));
    chk(vkResetFences(m_device, 1, &m_fences[m_frameIndex]));
    chkSwapchain(vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_presentSemaphores[m_frameIndex], VK_NULL_HANDLE, &m_imageIndex));

    m_currentCommandBuffer = m_commandBuffers[m_frameIndex];
    chk(vkResetCommandBuffer(m_currentCommandBuffer, 0));

    VkCommandBufferBeginInfo cbBI {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    chk(vkBeginCommandBuffer(m_currentCommandBuffer, &cbBI));

    imguiBeginRendering();
}

void Context::imguiBeginRendering() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Context::imguiEndRendering() {
    VkRenderingAttachmentInfo colorAttachment{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = getSwapchainImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,//VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = { .color = {{0, 0, 0, 1}} }
    };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { .extent = {
            .width = static_cast<uint32_t>(m_framebufferWidth),
            .height = static_cast<uint32_t>(m_framebufferHeight)
        }},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    // ImGui::ShowDemoWindow();
    ImGui::EndFrame();

    ImGui::Render();

    vkCmdBeginRendering(getCommandBuffer(), &renderingInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), getCommandBuffer());
    vkCmdEndRendering(getCommandBuffer());

}

void Context::endRendering() {
    imguiEndRendering();

    VkImageMemoryBarrier2 barrierPresent{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,// VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Can't use VK_IMAGE_LAYOUT_GENERAL here
        .image = m_swapchainImages[m_imageIndex],
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    };
    VkDependencyInfo barrierPresentDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrierPresent
    };
    vkCmdPipelineBarrier2(m_currentCommandBuffer, &barrierPresentDependencyInfo);

    vkEndCommandBuffer(m_currentCommandBuffer);


    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_presentSemaphores[m_frameIndex],
        .pWaitDstStageMask = &waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_currentCommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_renderSemaphores[m_imageIndex],
    };
    chk(vkQueueSubmit(m_queue, 1, &submitInfo, m_fences[m_frameIndex]));

    m_frameIndex = (m_frameIndex + 1) % maxFramesInFlight;

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_renderSemaphores[m_imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &m_imageIndex
    };
    chkSwapchain(vkQueuePresentKHR(m_queue, &presentInfo));
}

void Context::initOneTimeCommand() {
    VkFence fenceOneTime{};
    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };
    chk(vkCreateFence(m_device, &fenceCI, nullptr, &m_oneTimeFence));

    VkCommandBufferAllocateInfo commandBufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_commandPool,
        .commandBufferCount = 1
    };
    chk(vkAllocateCommandBuffers(m_device, &commandBufferAI, &m_oneTimeCommandBuffer));
}

void Context::doOneTimeCommand(std::function<void(VkCommandBuffer)> func) const {
    VkCommandBufferBeginInfo commandBufferBI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    chk(vkBeginCommandBuffer(m_oneTimeCommandBuffer, &commandBufferBI));

    func(m_oneTimeCommandBuffer);

    chk(vkEndCommandBuffer(m_oneTimeCommandBuffer));
    VkSubmitInfo oneTimeSI{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_oneTimeCommandBuffer
    };
    chk(vkQueueSubmit(m_queue, 1, &oneTimeSI, m_oneTimeFence));
    chk(vkWaitForFences(m_device, 1, &m_oneTimeFence, VK_TRUE, UINT64_MAX));
    chk(vkResetFences(m_device, 1, &m_oneTimeFence));
}
