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
    // GraphicsPipelineBuilder& setRasterization();
    // GraphicsPipelineBuilder& setDepthStencil();
    // GraphicsPipelineBuilder& setRenderingInfo();
    // GraphicsPipelineBuilder& setAddOpaqueAttachment();
    // GraphicsPipelineBuilder& addDynamic();
    [[nodiscard]] Pipeline build(const Context& ctx);

    // set_shaders(vert_module, frag_module)
    // add_vertex_binding(0, sizeof(Vertex))
    // add_vertex_attribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
    // set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    // set_rasterization(VK_CULL_MODE_BACK_BIT)
    // set_depth_stencil(VK_TRUE, VK_TRUE)
    // set_rendering_info(VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT)
    // add_opaque_attachment()
    // add_dynamic(VK_DYNAMIC_STATE_VIEWPORT)
    // build(device, pipeline_layout);

private:
    std::string m_moduleName;
    std::string m_shaderPath;

    VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // TODO: make so we can have multiple
    VkVertexInputBindingDescription m_vertexInputBinding{};
    // std::vector<VkVertexInputBindingDescription> m_vertexInputBindings;

    std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;
};
