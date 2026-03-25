#include "GraphicsPipelineBuilder.hpp"
#include "Context.hpp"
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <iostream>

static inline void chk(VkResult result) {
	if (result != VK_SUCCESS) {
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setShaders(const char* moduleName, const char* path) {
    m_moduleName = std::string(moduleName);
    m_shaderPath = std::string(path);
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::addVertexBinding(uint32_t binding, uint32_t stride) {
    m_vertexInputBinding.binding = binding;
    m_vertexInputBinding.stride = stride;
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::addVertexAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset) {
    m_vertexAttributes.push_back({location, binding, format, offset});
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setTopology(VkPrimitiveTopology topology) {
    m_topology = topology;
    return *this;
}

// GraphicsPipelineBuilder& setRasterization();
// GraphicsPipelineBuilder& setDepthStencil();
// GraphicsPipelineBuilder& setRenderingInfo();
// GraphicsPipelineBuilder& setAddOpaqueAttachment();
// GraphicsPipelineBuilder& addDynamic();

Pipeline GraphicsPipelineBuilder::build(const Context& ctx) {

    Slang::ComPtr<slang::IModule> slangModule{
        ctx.m_slangSession->loadModuleFromSource(m_moduleName.c_str(), m_shaderPath.c_str(), nullptr, nullptr)
    };
    Slang::ComPtr<ISlangBlob> spirv;
    slangModule->getTargetCode(0, spirv.writeRef());

    VkShaderModuleCreateInfo shaderModuleCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv->getBufferSize(),
        .pCode = (uint32_t*)spirv->getBufferPointer()
    };

    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = sizeof(VkDeviceAddress)
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0, // 1
        // .pSetLayouts = &descriptorSetLayoutTex,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
    chk(vkCreatePipelineLayout(ctx.m_device, &pipelineLayoutCI, nullptr, &pipelineLayout));

    VkVertexInputBindingDescription vertexBinding{
        .binding = m_vertexInputBinding.binding,
        .stride = m_vertexInputBinding.stride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexBinding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vertexAttributes.size()),
        .pVertexAttributeDescriptions = m_vertexAttributes.data(),
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = m_topology
    };

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
    {   .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &shaderModuleCI,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = VK_NULL_HANDLE,
        .pName = "main"
    },
    {   .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &shaderModuleCI,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = VK_NULL_HANDLE,
        .pName = "main" }
    };

    // TODO: make this optional with add_dynamic()
    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };
    std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates.data()
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
    };

    // TODO: make image format depends on custom input from setAddOpaqueAttachment
    const VkFormat imageFormat{ VK_FORMAT_B8G8R8A8_SRGB };
    VkPipelineRenderingCreateInfo renderingCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &imageFormat,
        .depthAttachmentFormat = ctx.m_depthFormat
    };

    VkPipelineColorBlendAttachmentState blendAttachment{
        .colorWriteMask = 0xF
    };
    VkPipelineColorBlendStateCreateInfo colorBlendState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &blendAttachment
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .lineWidth = 1.0f
    };
    VkPipelineMultisampleStateCreateInfo multisampleState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCI,
        .stageCount = 2,
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout
    };

    VkPipeline pipeline{ VK_NULL_HANDLE };
    chk(vkCreateGraphicsPipelines(ctx.m_device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline));

    return {pipeline, pipelineLayout};
}
