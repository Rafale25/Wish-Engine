#pragma once

#include <vulkan/vulkan.h>
#include "Context.hpp"
#include "Buffer.hpp"
#include <array>

class UniformBuffer {
public:
    void create(VkDeviceSize size);
    void upload(const void* data, size_t size);
    void pushConstant(VkPipelineLayout pipelineLayout, VkShaderStageFlagBits shaderStages);

private:
    std::array<Buffer, Context::maxFramesInFlight> m_shaderDataBuffers;
};
