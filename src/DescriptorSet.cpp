#include "Context.hpp"
#include "DescriptorSet.hpp"

DescriptorSet::~DescriptorSet() {
    const auto& device = Context::instance().getDevice();

    vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
}


void DescriptorSet::create(uint32_t textureCount) {
    const Context& ctx = Context::instance();

    VkDescriptorBindingFlags descVariableFlag{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
    VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 1,
        .pBindingFlags = &descVariableFlag
    };
    VkDescriptorSetLayoutBinding descLayoutBinding{
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = textureCount,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutCreateInfo descSetLayoutCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &descBindingFlags,
        .bindingCount = 1,
        .pBindings = &descLayoutBinding
    };

    vkCreateDescriptorSetLayout(ctx.getDevice(), &descSetLayoutCI, nullptr, &m_descriptorSetLayout);

    VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = textureCount
    };
    VkDescriptorPoolCreateInfo descPoolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    vkCreateDescriptorPool(ctx.getDevice(), &descPoolCI, nullptr, &m_descriptorPool);

    uint32_t variableDescCount{ textureCount };
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
        .descriptorSetCount = 1,
        .pDescriptorCounts = &variableDescCount
    };
    VkDescriptorSetAllocateInfo descSetAlloc{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variableDescCountAI,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descriptorSetLayout
    };

    vkAllocateDescriptorSets(ctx.getDevice(), &descSetAlloc, &m_descriptorSet);
}

int32_t DescriptorSet::addTexture(VkSampler sampler, VkImageView imageView) {
    m_imageDescriptors.push_back({
        .sampler = sampler,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    });

    VkWriteDescriptorSet writeDescSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descriptorSet,
        .dstBinding = 0,
        .descriptorCount = static_cast<uint32_t>(m_imageDescriptors.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = m_imageDescriptors.data()
    };

    vkUpdateDescriptorSets(Context::instance().getDevice(), 1, &writeDescSet, 0, nullptr);

    return m_imageDescriptors.size() - 1;
}

void DescriptorSet::bind(VkCommandBuffer cb, VkPipelineLayout pipelineLayout) {
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
}
