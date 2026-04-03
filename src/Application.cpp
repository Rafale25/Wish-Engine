#include "Context.hpp"
#include "Application.hpp"
#include "UniformBuffer.hpp"
#include "RenderPass.hpp"
#include <glm/ext/vector_float3.hpp>
#include <print>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

App::~App() {
    m_pipeline.destroy();
    gigachad.destroy();
}

App::App() {
    const Context& ctx = Context::instance();

    m_uniformBuffer.create(sizeof(ShaderData));

    gigachad.createFromFile("Gigachad.jpg");
    gigachad.createSampler();

    m_descriptorSet.create(0, 1);
    int32_t textureIndex = m_descriptorSet.addTexture(gigachad.sampler, gigachad.imageView);

    m_pipeline = GraphicsPipelineBuilder{}
        .setShaders("triangle", "./src/shader.slang")
        .addColorAttachmentFormat(Context::SWAPCHAIN_IMAGE_FORMAT)
        .setDepthAttachmentFormat(ctx.getDepthImageFormat())
        .addVertexBinding(0, sizeof(Vertex))
        .addDescriptorLayout(m_descriptorSet.getLayout())
        .addVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
        .addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
        .addVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv))
        .setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .setPolygonMode(VK_POLYGON_MODE_FILL)
        .build();

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
    m_shaderData.time = time_since_start;
}

void App::onDraw(double time_since_start, float dt) {
    VkCommandBuffer cb = Context::instance().getCommandBuffer();
    Context& ctx = Context::instance();

    m_uniformBuffer.upload(&m_shaderData, sizeof(ShaderData));

    auto pass = RenderPass()
        .defaultViewportScissor()
        .color(
            ctx.getSwapchainImage(),
            ctx.getSwapchainImageView())
        .depth(
            ctx.getDepthTexture());

    pass.execute([&]() {
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.pipeline);
        m_descriptorSet.bind(cb, m_pipeline.layout);

        VkDeviceSize _vOffset{ 0 };
        vkCmdBindVertexBuffers(cb, 0, 1, &m_bufferVertex.buffer, &_vOffset);
        vkCmdBindIndexBuffer(cb, m_bufferIndices.buffer, 0, VK_INDEX_TYPE_UINT16);

        m_uniformBuffer.pushConstant(m_pipeline.layout, VkShaderStageFlagBits(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));


        const VkDeviceSize indexCount{6};
        vkCmdDrawIndexed(cb, indexCount, 1, 0, 0, 0);
    });
}

void App::onResize(int width, int height) {

}
