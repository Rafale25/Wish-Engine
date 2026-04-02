#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include "View.hpp"
#include "Texture.hpp"
#include <array>
#include <vector>
#include <string>
#include <functional>

struct GLFWwindow;

class Context {
private:
    Context() = default;
    ~Context();// = default;

    Context(const Context&) = delete;
    Context(Context&&)      = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&)      = delete;

public:
    static Context& instance() {
        static Context ctx{};
        return ctx;
    }

    void init();
    void run();
    void setView(View& view);

    void doOneTimeCommand(std::function<void(VkCommandBuffer)>) const;

    const VmaAllocator& getVmaAllocator() const { return m_allocator; }
    const Slang::ComPtr<slang::ISession>& getSlangSession() const { return m_slangSession; }
    VkCommandBuffer getCommandBuffer() const { return m_commandBuffers[m_frameIndex]; }
    VkCommandPool getCommandPool() const { return m_commandPool; };
    uint32_t getFrameIndex() const { return m_frameIndex; }
    VkDevice getDevice() const { return m_device; }
    VkFence getFence() const { return m_fences[m_frameIndex]; }
    VkQueue getQueue() const { return m_queue; }
    VkImage getSwapchainImage() const { return m_swapchainImages[m_imageIndex]; }
    VkImageView getSwapchainImageView() const { return m_swapchainImageViews[m_imageIndex]; }
    VkImage getDepthImage() const { return m_depthTexture.image; }
    VkImageView getDepthImageView() const { return m_depthTexture.imageView; }
    VkFormat getDepthImageFormat() const { return m_depthFormat; }
    uint32_t width() const { return static_cast<uint32_t>(m_framebufferWidth); };
    uint32_t height() const { return static_cast<uint32_t>(m_framebufferHeight); };

private:
    void initWindow();
    void updateSwapchain();
    void chkSwapchain(VkResult result);

    void beginRendering();
    void endRendering();

    void initOneTimeCommand();

    void createDebugUtilsMessenger();
    void destroyDebugUtilsMessenger();
    void cleanup();

    // static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    // static void cursorPositionCallback(GLFWwindow* window, double x, double y);
    // static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    // static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    // static void cursorEnterCallback(GLFWwindow* window, int entered);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

public:
    // GLFW
    GLFWwindow* window = nullptr;
    int32_t m_framebufferWidth = 0;
    int32_t m_framebufferHeight = 0;
    double m_timeSinceStart = 0.0;

    // Vulkan
    static constexpr uint32_t API_VERSION = VK_API_VERSION_1_4;
    static constexpr uint32_t maxFramesInFlight{ 2 }; // Will be configurable at runtime in the futur
    static constexpr VkFormat SWAPCHAIN_IMAGE_FORMAT{ VK_FORMAT_B8G8R8A8_SRGB };

    #ifdef NDEBUG
        static constexpr bool ENABLE_VALIDATION_LAYERS = false;
    #else
        static constexpr bool ENABLE_VALIDATION_LAYERS = true;
        VkDebugUtilsMessengerEXT m_logger{ nullptr };
    #endif

private:
    // GLFW
    DefaultView m_defaultView;
    View* m_currentView = &m_defaultView;
    std::string applicationName = "Vulkan Application";

    const std::vector<char const*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Vulkan
    VkInstance m_instance{ VK_NULL_HANDLE };
    std::vector<VkPhysicalDevice> m_devices;
    uint32_t m_deviceCount{ 0 };
    uint32_t m_deviceIndex{ 0 };
    VkDevice m_device{ VK_NULL_HANDLE };
    VkQueue m_queue{ VK_NULL_HANDLE };
    VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
    VkSurfaceCapabilitiesKHR m_surfaceCaps{};
    VmaAllocator m_allocator{ VK_NULL_HANDLE };
    VkCommandPool m_commandPool{ VK_NULL_HANDLE };

    std::array<VkCommandBuffer, maxFramesInFlight> m_commandBuffers;
    std::array<VkFence, maxFramesInFlight> m_fences;
    std::array<VkSemaphore, maxFramesInFlight> m_presentSemaphores;
    std::vector<VkSemaphore> m_renderSemaphores;

    VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
    uint32_t m_swapchainImageCount{ 0 };
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    bool m_updateSwapchain{ false };

    Slang::ComPtr<slang::IGlobalSession> m_slangGlobalSession;
    Slang::ComPtr<slang::ISession> m_slangSession;

    // Render loop
    uint32_t m_frameIndex{ 0 };
    uint32_t m_imageIndex{ 0 };
    VkCommandBuffer_T* m_currentCommandBuffer{ nullptr };

    // One time commands
    VkFence m_oneTimeFence { VK_NULL_HANDLE };
    VkCommandBuffer m_oneTimeCommandBuffer { VK_NULL_HANDLE };


    // TODO: move everything belowout of Context
    // this should be user data and not generic
    VkSwapchainCreateInfoKHR m_swapchainCI;

    Texture m_depthTexture{};
    VkFormat m_depthFormat{ VK_FORMAT_UNDEFINED };
};
