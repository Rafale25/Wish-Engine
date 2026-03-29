#include "Texture.hpp"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "Context.hpp"

void Texture::create(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags imageUsageFlags, VkImageAspectFlags viewAspectMask) {
    const Context& ctx = Context::instance();

    imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent{.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = imageUsageFlags,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    vmaCreateImage(ctx.getVmaAllocator(), &imageCI, &allocCI, &image, &allocation, nullptr);

    imageViewCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange{ .aspectMask = viewAspectMask, .levelCount = 1, .layerCount = 1 }
    };
    vkCreateImageView(ctx.getDevice(), &imageViewCI, nullptr, &imageView);
}

void Texture::destroy() {
    const auto& ctx = Context::instance();

    vkDestroyImageView(ctx.getDevice(), imageView, nullptr);
    vmaDestroyImage(ctx.getVmaAllocator(), image, allocation);

    allocation = nullptr;
    image = nullptr;
    imageCI = {};
    imageView = nullptr;
    imageViewCI = {};
}
