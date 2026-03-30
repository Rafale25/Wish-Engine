#include "Texture.hpp"
#include "Buffer.hpp"
#include "Context.hpp"
#include "Logger.hpp"
#include "stb_image.h"

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

void Texture::createFromFile(const char* path) {
    int32_t width, height, channels;
    int r = stbi_info(path, &width, &height, &channels);

    if (r != 1)
        logE("Couldn't load file: {}", path);

    create(
        VK_FORMAT_R8G8B8A8_SRGB,
        width, height,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &channels, 4);
    int32_t dataSize = width * height * 4;

    const auto& ctx = Context::instance();
    const auto fence = ctx.getFence();

    Buffer stagingBuffer;
    stagingBuffer.create(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stagingBuffer.upload(data, dataSize);

    const int numLevels = 1;

    ctx.doOneTimeCommand([&](VkCommandBuffer cb) {
        VkImageMemoryBarrier2 barrierTexImage{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .image = this->image,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = 1 }
        };
        VkDependencyInfo barrierTexInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrierTexImage
        };
        vkCmdPipelineBarrier2(cb, &barrierTexInfo);

        // TODO: add mipmap generator
        std::vector<VkBufferImageCopy> copyRegions{};
        for (uint32_t i = 0; i < numLevels; ++i) {
            VkDeviceSize offset = 0;
            copyRegions.push_back({
                .bufferOffset = offset,
                .imageSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = (uint32_t)i, .layerCount = 1},
                .imageExtent{.width = static_cast<uint32_t>(width) >> i, .height = static_cast<uint32_t>(height) >> i, .depth = 1 },
            });
        }
        vkCmdCopyBufferToImage(cb, stagingBuffer.buffer, this->image, VK_IMAGE_LAYOUT_GENERAL, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

        VkImageMemoryBarrier2 barrierTexRead{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .image = this->image,
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = 1 }
        };
        barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
        vkCmdPipelineBarrier2(cb, &barrierTexInfo);
    });
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
