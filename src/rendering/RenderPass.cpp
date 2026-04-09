#include "RenderPass.hpp"
#include "Texture.hpp"
#include "Context.hpp"

RenderPass& RenderPass::setExtents(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return *this;
}

RenderPass& RenderPass::color(const Texture& texture, ColorAttachmentInfo attachmentInfo, VkImageMemoryBarrier2 barrier) {
    return color(texture.image, texture.imageView, attachmentInfo, barrier);
}

RenderPass& RenderPass::color(VkImage image, VkImageView imageView, ColorAttachmentInfo attachmentInfo, VkImageMemoryBarrier2 barrier) {
    barrier.image = image;

    VkRenderingAttachmentInfo renderingAttachmentInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .loadOp = attachmentInfo.loadOp,
        .storeOp = attachmentInfo.storeOp,
        .clearValue = { attachmentInfo.clearValue }
    };

    m_colorAttachments.push_back(renderingAttachmentInfo);
    m_barriers.push_back(barrier);
    return *this;
}

RenderPass& RenderPass::depth(const Texture& texture, DepthAttachmentInfo attachmentInfo, VkImageMemoryBarrier2 barrier) {
    return depth(texture.image, texture.imageView, attachmentInfo, barrier);
}

RenderPass& RenderPass::depth(VkImage image, VkImageView imageView, DepthAttachmentInfo attachmentInfo, VkImageMemoryBarrier2 barrier) {
    barrier.image = image;

    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .loadOp = attachmentInfo.loadOp,
        .storeOp = attachmentInfo.storeOp,
        .clearValue = attachmentInfo.clearValue
    };

    m_barriers.push_back(barrier);
    m_depthAttachment = depthAttachmentInfo;
    return *this;
}

RenderPass& RenderPass::defaultViewportScissor() {
    doDefaultViewportScissor = true;
    return *this;
}

void RenderPass::beginRendering() {
    const Context& ctx = Context::instance();
    const auto cb = ctx.getCommandBuffer();

    if (m_width == 0 && m_height == 0) {
        m_width = ctx.m_framebufferWidth;
        m_height = ctx.m_framebufferHeight;
    }

    VkDependencyInfo barrierDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(m_barriers.size()),
        .pImageMemoryBarriers = m_barriers.data()
    };
    vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{.extent{.width = m_width, .height = m_height }},
        .layerCount = 1,
        .colorAttachmentCount = static_cast<uint32_t>(m_colorAttachments.size()),
        .pColorAttachments = m_colorAttachments.data(),
        .pDepthAttachment = &m_depthAttachment
    };
    vkCmdBeginRendering(cb, &renderingInfo);

    if (doDefaultViewportScissor) {
        VkViewport vp{
            .x = 0.0f,
            .y = 0.0f, //static_cast<float>(ctx.height()),  // Start at bottom
            .width = static_cast<float>(ctx.width()),
            .height = static_cast<float>(ctx.height()),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        VkRect2D scissor{ .extent{
            .width = static_cast<uint32_t>(ctx.width()),
            .height = static_cast<uint32_t>(ctx.height()) }
        };
        vkCmdSetViewport(cb, 0, 1, &vp);
        vkCmdSetScissor(cb, 0, 1, &scissor);
    }
}

void RenderPass::endRendering() {
    vkCmdEndRendering(Context::instance().getCommandBuffer());
}

void RenderPass::execute(std::function<void()> func) {
    beginRendering();
    func();
    endRendering();
}
