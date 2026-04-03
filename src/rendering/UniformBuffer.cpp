#include "UniformBuffer.hpp"
#include <vk_mem_alloc.h>

void UniformBuffer::create(VkDeviceSize size) {
    for (uint32_t i = 0; i < Context::maxFramesInFlight; ++i) {
        m_shaderDataBuffers[i].create(size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    }
}

void UniformBuffer::upload(const void* data, size_t size) {
    auto frameIndex = Context::instance().getFrameIndex();
    memcpy(m_shaderDataBuffers[frameIndex].allocationInfo.pMappedData, data, size);
}

void UniformBuffer::pushConstant(VkPipelineLayout pipelineLayout, VkShaderStageFlagBits shaderStages) {
    const auto& ctx = Context::instance();
    vkCmdPushConstants(
        ctx.getCommandBuffer(),
        pipelineLayout,
        shaderStages,
        0, sizeof(VkDeviceAddress),
        &m_shaderDataBuffers[ctx.getFrameIndex()].deviceAddress);
}
