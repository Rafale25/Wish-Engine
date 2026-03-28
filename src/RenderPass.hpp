#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <functional>

static const VkRenderingAttachmentInfo defaultColorAttachmentInfo {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = nullptr,
    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue{.color{ {0.0f, 0.0f, 0.0f, 1.0f} }}
};

static const VkImageMemoryBarrier2 defaultColorBarrier{
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .newLayout = VK_IMAGE_LAYOUT_GENERAL,
    .image = nullptr,
    .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
};

static const VkRenderingAttachmentInfo defaultDepthAttachmentInfo{
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = nullptr,
    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .clearValue = {.depthStencil = {1.0f,  0}}
};

static const VkImageMemoryBarrier2 defaultDepthBarrier={
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
    .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
    .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .newLayout = VK_IMAGE_LAYOUT_GENERAL,
    .image = nullptr,
    .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .levelCount = 1, .layerCount = 1 }
};

class RenderPass {
public:
    RenderPass& setExtents(uint32_t width, uint32_t height);

    RenderPass& color(
        VkImage image, VkImageView imageView,
        VkRenderingAttachmentInfo attachmentInfo=defaultColorAttachmentInfo,
        VkImageMemoryBarrier2 barrier=defaultColorBarrier);

    RenderPass& depth(
        VkImage image, VkImageView imageView,
        VkRenderingAttachmentInfo attachmentInfo=defaultDepthAttachmentInfo,
        VkImageMemoryBarrier2 barrier=defaultDepthBarrier);

    RenderPass& defaultViewportScissor();

    void beginRendering();
    void endRendering();
    void execute(std::function<void()> func);

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<VkImageMemoryBarrier2> m_barriers;
    std::vector<VkRenderingAttachmentInfo> m_colorAttachments;
    VkRenderingAttachmentInfo m_depthAttachment{};
    bool doDefaultViewportScissor = false;
};
