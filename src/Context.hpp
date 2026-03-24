#pragma once

#include <vma/vk_mem_alloc.h>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <glm/glm.hpp>
#include "View.hpp"
#include <array>
#include <vector>
#include <string>

struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    float time{0.0f};
};

struct ShaderDataBuffer {
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VmaAllocationInfo allocationInfo{};
    VkBuffer buffer{ VK_NULL_HANDLE };
    VkDeviceAddress deviceAddress{};
};

struct GLFWwindow;

class Context {
public:
    Context() = default;
    ~Context() = default;

    void init();
    void run();

    void setView(View& view);

private:
    void initWindow();
    void updateSwapchain();
    void chkSwapchain(VkResult result);

    void beginRendering();
    void endRendering();

    // static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    // static void cursorPositionCallback(GLFWwindow* window, double x, double y);
    // static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    // static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    // static void cursorEnterCallback(GLFWwindow* window, int entered);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

public:
    // GLFW
    GLFWwindow* window = nullptr;
    int m_framebufferWidth = 0;
    int m_framebufferHeight = 0;
    // int32_t width = 0, height = 0;

private:
    DefaultView m_defaultView{*this};
    View* m_currentView = &m_defaultView;
    std::string applicationName = "Vulkan Application";

    // Vulkan
    static constexpr uint32_t API_VERSION = VK_API_VERSION_1_4;
    static constexpr uint32_t maxFramesInFlight{ 2 }; // Will be configurable at runtime in the futur

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
    std::array<ShaderDataBuffer, maxFramesInFlight> m_shaderDataBuffers;

    VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
    static constexpr VkFormat SWAPCHAIN_IMAGE_FORMAT{ VK_FORMAT_B8G8R8A8_SRGB };
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


    // TODO: move everything belowout of Context
    // this should be user data and not generic
    ShaderData m_shaderData{};

    VkSwapchainCreateInfoKHR m_swapchainCI;

    VmaAllocation m_depthImageAllocation;
    VkFormat m_depthFormat{ VK_FORMAT_UNDEFINED };
    VkImageCreateInfo m_depthImageCI;
    VkImage m_depthImage;
    VkImageView m_depthImageView;
    VkImageViewCreateInfo m_depthViewCI;

};
