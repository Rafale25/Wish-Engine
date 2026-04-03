#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class DescriptorSet {
public:
    DescriptorSet() = default;
    ~DescriptorSet();

    void create(uint32_t binding, uint32_t textureCount);
    int32_t addTexture(VkSampler sampler, VkImageView imageView);
    void bind(VkCommandBuffer cb, VkPipelineLayout pipelineLayout);

    VkDescriptorSetLayout getLayout() const { return m_descriptorSetLayout; }

private:
    VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };
    VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
    VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE };
    std::vector<VkDescriptorImageInfo> m_imageDescriptors;
};
