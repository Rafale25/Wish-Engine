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
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // specifies that access to any range or image subresource of the object will be exclusive to a single queue family at a time
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

void Texture::createCubemap(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags imageUsageFlags, VkImageAspectFlags viewAspectMask) {
    const Context& ctx = Context::instance();

    imageCI = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, //
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent{.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 6,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = imageUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // specifies that access to any range or image subresource of the object will be exclusive to a single queue family at a time
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
        .viewType = VK_IMAGE_VIEW_TYPE_CUBE, //
        .format = format,
        .subresourceRange{ .aspectMask = viewAspectMask, .levelCount = 1, .layerCount = 6 }
    };
    vkCreateImageView(ctx.getDevice(), &imageViewCI, nullptr, &imageView);
}

void Texture::createSampler(VkFilter minFilter, VkFilter magFilter) {
    VkSamplerCreateInfo samplerCI{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = magFilter,
        .minFilter = minFilter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.0f, // 8 is a widely supported value for max anisotropy
        .maxLod = 1.0f, // numLevels
    };
    vkCreateSampler(Context::instance().getDevice(), &samplerCI, nullptr, &this->sampler);
}

void Texture::createFromFile(const char* path) {
    int32_t width, height, channels;
    int r = stbi_info(path, &width, &height, &channels);

    if (r != 1)
        logE("Couldn't load file: {}", path);

    // TODO: choose format depending on number of channels
    create(
        VK_FORMAT_R8G8B8A8_SRGB,
        width, height,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    int32_t dataSize = width * height * 4;

    const auto& ctx = Context::instance();

    Buffer stagingBuffer;
    stagingBuffer.create(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    stagingBuffer.upload(data, dataSize);

    stbi_image_free(data);

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
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
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

void Texture::createFromFileCubemap(const char* path[6]) {
    int32_t width, height, channels;
    int r = stbi_info(path[0], &width, &height, &channels);

    if (r != 1) {
        logE("Couldn't load file: {}", path[0]);
        exit(-1);
    }

    // TODO: choose format depending on number of channels
    createCubemap(
        VK_FORMAT_R8G8B8A8_SRGB,
        width, height,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    stbi_set_flip_vertically_on_load(false);

    Buffer stagingBuffer;
    int32_t dataSize = width * height * 4;
    int32_t totalDataSize = dataSize * 6;
    stagingBuffer.create(totalDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    for (int i = 0 ; i < 6 ; ++i) {
        unsigned char *data = stbi_load(path[i], &width, &height, &channels, STBI_rgb_alpha);
        stagingBuffer.upload(data, dataSize, dataSize*i);
        stbi_image_free(data);
    }

    const int numLevels = 1;
    const uint32_t layerCount = 6;

    const auto& ctx = Context::instance();
    ctx.doOneTimeCommand([&](VkCommandBuffer cb) {
        VkImageMemoryBarrier2 barrierTexImage{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image = this->image,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = layerCount }
        };
        VkDependencyInfo barrierTexInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrierTexImage
        };
        vkCmdPipelineBarrier2(cb, &barrierTexInfo);

        std::vector<VkBufferImageCopy> copyRegions{};
        for (uint32_t face = 0 ; face < 6 ; ++face) {
            for (uint32_t level = 0; level < numLevels; ++level) {
                VkDeviceSize offset = dataSize * face;
                copyRegions.push_back({
                    .bufferOffset = offset,
                    .imageSubresource{
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = (uint32_t)level,
                        .baseArrayLayer = face,
                        .layerCount = 1
                    },
                    .imageExtent{
                        .width = static_cast<uint32_t>(width) >> level,
                        .height = static_cast<uint32_t>(height) >> level,
                        .depth = 1
                    },
                });
            }
        }

        vkCmdCopyBufferToImage(cb, stagingBuffer.buffer, this->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

        VkImageMemoryBarrier2 barrierTexRead{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .image = this->image,
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = layerCount }
        };
        barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
        vkCmdPipelineBarrier2(cb, &barrierTexInfo);
    });
}

void Texture::destroy() {
    if (image == nullptr)
        return;

    const auto& ctx = Context::instance();

    vkDestroyImageView(ctx.getDevice(), imageView, nullptr);
    vkDestroySampler(ctx.getDevice(), sampler, nullptr);
    vmaDestroyImage(ctx.getVmaAllocator(), image, allocation);

    allocation = nullptr;
    image = nullptr;
    imageCI = {};
    imageView = nullptr;
    imageViewCI = {};
}
