#pragma once

#include <vulkan/vulkan.h>

struct Pipeline {
    void destroy();

    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout layout{ VK_NULL_HANDLE };
};
