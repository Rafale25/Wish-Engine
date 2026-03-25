#include "Context.hpp"
#include "Application.hpp"

#include <print>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

App::App(Context& ctx): View(ctx) {

    m_pipeline = GraphicsPipelineBuilder{}
        .setShaders("triangle", "./src/shader.slang")
        .addVertexBinding(0, sizeof(Vertex))
        .addVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
        .addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
        .addVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv))
        .setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .setPolygonMode(VK_POLYGON_MODE_FILL)
        .addDynamic(VK_DYNAMIC_STATE_VIEWPORT)
        .addDynamic(VK_DYNAMIC_STATE_SCISSOR)
        .build();

/*
    VkPipeline templatePipeline = GraphicsPipelineBuilder{}
        .set_rasterization(VK_CULL_MODE_BACK_BIT)
        .set_depth_stencil(VK_TRUE, VK_TRUE)
        .set_rendering_info(VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT)
        .add_opaque_attachment()
        .build(device, pipeline_layout);
*/

    const VkDeviceSize indexCount{6};
    std::vector<Vertex> vertices{
        {{-1,  1, 0}, {1, 0, 0}, {0, 0}},
        {{ 1,  1, 0}, {0, 1, 0}, {1, 0}},
        {{-1, -1, 0}, {0, 0, 1}, {0, 1}},
        {{ 1, -1, 0}, {0, 1, 1}, {1, 1}},
    };
    std::vector<uint16_t> indices{
        0, 1, 2,
        2, 1, 3,
    };

    int verticesSize = vertices.size() * sizeof(Vertex);
    int indicesSize = indices.size() * sizeof(uint16_t);

    m_bufferVertex.create(verticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_bufferIndices.create(indicesSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    m_bufferVertex.upload(vertices.data(), verticesSize);
    m_bufferIndices.upload(indices.data(), indicesSize);
}

void App::onUpdate(double time_since_start, float dt) {

}

void App::onDraw(double time_since_start, float dt) {
    VkCommandBuffer cb = Context::instance().getCommandBuffer();
    Context& ctx = Context::instance();

    VkViewport vp{
        .width = static_cast<float>(ctx.m_framebufferWidth),
        .height = static_cast<float>(ctx.m_framebufferHeight),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor{ .extent{
        .width = static_cast<uint32_t>(ctx.m_framebufferWidth),
        .height = static_cast<uint32_t>(ctx.m_framebufferHeight) }
    };
    vkCmdSetViewport(cb, 0, 1, &vp);
    vkCmdSetScissor(cb, 0, 1, &scissor);


    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.pipeline);
        VkDeviceSize vOffset{ 0 };
        vkCmdBindVertexBuffers(cb, 0, 1, &m_bufferVertex.get(), &vOffset);
        vkCmdBindIndexBuffer(cb, m_bufferIndices.get(), 0, VK_INDEX_TYPE_UINT16);

        // vkCmdSetLineWidth(cb, 10.0f);

        vkCmdPushConstants(cb, m_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &ctx.m_shaderDataBuffers[ctx.m_frameIndex].deviceAddress);

        const VkDeviceSize indexCount{6};
        vkCmdDrawIndexed(cb, indexCount, 1, 0, 0, 0);
}

void App::onResize(int width, int height) {

}
