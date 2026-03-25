#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class Context;

struct Pipeline {
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout layout{ VK_NULL_HANDLE };
};

class GraphicsPipelineBuilder {
public:

    GraphicsPipelineBuilder& setShaders(const char* moduleName, const char* path);
    GraphicsPipelineBuilder& addVertexBinding(uint32_t binding, uint32_t stride);
    GraphicsPipelineBuilder& addVertexAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);
    GraphicsPipelineBuilder& setTopology(VkPrimitiveTopology topology);
    GraphicsPipelineBuilder& setPolygonMode(VkPolygonMode polygonMode);
    GraphicsPipelineBuilder& setCullMode(VkCullModeFlags flags);
    GraphicsPipelineBuilder& setFrontFace(VkFrontFace orientation);
    GraphicsPipelineBuilder& setDepthCompareOp(VkCompareOp op);
    GraphicsPipelineBuilder& setColorWriteMask(VkColorComponentFlags flags);
    GraphicsPipelineBuilder& addDynamic(VkDynamicState dynamicState);

    [[nodiscard]] Pipeline build();

    // GraphicsPipelineBuilder& setRenderingInfo();
    // set_rendering_info(VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT)

private:
    std::string m_moduleName;
    std::string m_shaderPath;

    VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags m_cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace m_frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkCompareOp m_compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    VkColorComponentFlags m_colorWriteMask = 0xF;

    std::vector<VkDynamicState> m_dynamicStates;

    // TODO: make so we can have multiple
    VkVertexInputBindingDescription m_vertexInputBinding{};
    // std::vector<VkVertexInputBindingDescription> m_vertexInputBindings;

    std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;
};
