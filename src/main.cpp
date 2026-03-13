#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>

#include <iostream>
#include <vector>

static inline void chk(VkResult result) {
	if (result != VK_SUCCESS) {
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Wish Engine",
        .apiVersion = VK_API_VERSION_1_3
    };

    uint32_t instanceExtensionsCount{ 0 };
    char const* const* instanceExtensions{ glfwGetRequiredInstanceExtensions(&instanceExtensionsCount) };

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = instanceExtensionsCount,
        .ppEnabledExtensionNames = instanceExtensions,
    };

    VkInstance instance{ VK_NULL_HANDLE };
    chk(vkCreateInstance(&instanceCI, nullptr, &instance));

    uint32_t deviceCount{ 0 };
    chk(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    chk(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

    std::print("deviceCount : {}", deviceCount);

    uint32_t deviceIndex{ 0 };

    VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &deviceProperties);
    std::cout << "Selected device: " << deviceProperties.properties.deviceName <<  "\n";


    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamily{ 0 };
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamily = i;
            break;
        }
    }

    if (glfwGetPhysicalDevicePresentationSupport(instance, devices[deviceIndex], queueFamily) == GLFW_FALSE) {
		std::cerr << "Graphic queue doesn't support presentation!\n";
		exit(-1);
    }


    const float qfpriorities{ 1.0f };
    VkDeviceQueueCreateInfo queueCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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
    const VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledVk12Features,
        .synchronization2 = true,
        .dynamicRendering = true,
    };

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features
    };

    VkDevice device{ VK_NULL_HANDLE };
    chk(vkCreateDevice(devices[deviceIndex], &deviceCI, nullptr, &device));

    VkQueue queue{ VK_NULL_HANDLE };
    vkGetDeviceQueue(device, queueFamily, 0, &queue);


    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        /* Render here */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
