#include "RenderPass.hpp"
#include "Context.hpp"

RenderPass& RenderPass::setExtents(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return *this;
}

RenderPass& RenderPass::color(VkImage image, VkImageView imageView, VkRenderingAttachmentInfo attachmentInfo, VkImageMemoryBarrier2 barrier) {
    barrier.image = image;
    attachmentInfo.imageView = imageView;
    m_barriers.push_back(barrier);
    m_colorAttachments.push_back(attachmentInfo);
    return *this;
}

RenderPass& RenderPass::depth(VkImage image, VkImageView imageView, VkRenderingAttachmentInfo attachmentInfo, VkImageMemoryBarrier2 barrier) {
    barrier.image = image;
    attachmentInfo.imageView = imageView;
    m_barriers.push_back(barrier);
    m_depthAttachment = attachmentInfo;
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
}

void RenderPass::endRendering() {
    vkCmdEndRendering(Context::instance().getCommandBuffer());
}

void RenderPass::execute(std::function<void()> func) {
    beginRendering();
    func();
    endRendering();
}
