#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "Context.hpp"

struct ShaderDataBuffer {
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VmaAllocationInfo allocationInfo{};
    VkBuffer buffer{ VK_NULL_HANDLE };
    VkDeviceAddress deviceAddress{};
};

class UniformBuffer {
public:
    void create(VkDeviceSize size) {
        const Context& ctx = Context::instance();

        for (uint32_t i = 0; i < Context::maxFramesInFlight; ++i) {
            VkBufferCreateInfo uBufferCI{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = size,
                .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            };
            VmaAllocationCreateInfo uBufferAllocCI{
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };
            vmaCreateBuffer(
                ctx.getVmaAllocator(),
                &uBufferCI, &uBufferAllocCI,
                &m_shaderDataBuffers[i].buffer,
                &m_shaderDataBuffers[i].allocation,
                &m_shaderDataBuffers[i].allocationInfo);

            VkBufferDeviceAddressInfo uBufferBdaInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = m_shaderDataBuffers[i].buffer
            };
            m_shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(ctx.getDevice(), &uBufferBdaInfo);
        }
    }

    void upload(const void* data, size_t size) {
        auto frameIndex = Context::instance().getFrameIndex();
        memcpy(m_shaderDataBuffers[frameIndex].allocationInfo.pMappedData, data, size);
    }

    void pushConstant(VkPipelineLayout pipelineLayout, VkShaderStageFlagBits shaderStages) {
        auto frameIndex = Context::instance().getFrameIndex();
        VkCommandBuffer commandBuffer = Context::instance().getCommandBuffer();

        vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            shaderStages,
            0, sizeof(VkDeviceAddress),
            &m_shaderDataBuffers[frameIndex].deviceAddress);
    }

private:
    std::array<ShaderDataBuffer, Context::maxFramesInFlight> m_shaderDataBuffers;
};
