#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

struct Texture {
    void create(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags imageUsageFlags, VkImageAspectFlags viewAspectMask);
    void createFromFile(const char* path);
    void createSampler(VkFilter minFilter=VK_FILTER_LINEAR, VkFilter magFilter=VK_FILTER_LINEAR);
    void destroy();

    VmaAllocation allocation = nullptr;
    VkImage image = nullptr;
    VkImageCreateInfo imageCI{};
    VkImageView imageView = nullptr;
    VkImageViewCreateInfo imageViewCI{};
    VkSampler sampler = nullptr;
};
