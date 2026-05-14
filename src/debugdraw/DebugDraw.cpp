#include "DebugDraw.hpp"
#include "RenderPass.hpp"
#include "Context.hpp"
#include "GraphicsPipelineBuilder.hpp"
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan.h>

static inline int32_t packColor(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16 | g << 8 | b);
}

DebugDraw::DebugDraw() {
    const auto& ctx = Context::instance();

    m_uniformBuffer.create(sizeof(DebugDrawShaderData));

    constexpr int initial_size = 10'000 * sizeof(float) * 4;

    m_vertexBuffer.create(initial_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    m_pipeline = GraphicsPipelineBuilder{}
        .setShaders("debugDraw", "./src/shaders/debugdraw.slang")
        .addColorAttachmentFormat(Context::SWAPCHAIN_IMAGE_FORMAT)
        .setDepthAttachmentFormat(ctx.getDepthImageFormat())
        .addVertexBinding(0, sizeof(DebugDrawVertex))
        .addVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DebugDrawVertex, pos))
        .addVertexAttribute(1, 0, VK_FORMAT_R32_SINT,         offsetof(DebugDrawVertex, color))
        .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
        .setCullMode(VK_CULL_MODE_NONE)
        // .setPolygonMode(VK_POLYGON_MODE_LINE)
        // .addDynamic(VK_DYNAMIC_STATE_LINE_WIDTH)
        .build();
}

DebugDraw::~DebugDraw() {
    m_pipeline.destroy();
}

void DebugDraw::drawLine(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &color) {
    m_vertices.emplace_back(a, packColor(color.r * 255, color.g * 255, color.b * 255));
    m_vertices.emplace_back(b, packColor(color.r * 255, color.g * 255, color.b * 255));
}

void DebugDraw::drawRay(const glm::vec3 &start, const glm::vec3 &v, const glm::vec3 &color) {
    drawLine(start, start + v, color);
}

void DebugDraw::drawCube(const glm::vec3 &center, float size, const glm::vec3 &color) {
    drawCuboid(center, glm::vec3(size*0.5f), color);
}

void DebugDraw::drawCuboid(const glm::vec3 &center, const glm::vec3 &extents, const glm::vec3 &color) {
    // -Y
    const glm::vec3 x0y0z0 = center + glm::vec3{-extents.x, -extents.y, -extents.z};
    const glm::vec3 x1y0z0 = center + glm::vec3{extents.x, -extents.y, -extents.z};
    const glm::vec3 x0y0z1 = center + glm::vec3{-extents.x, -extents.y, extents.z};
    const glm::vec3 x1y0z1 = center + glm::vec3{extents.x, -extents.y, extents.z};

    // +Y
    const glm::vec3 x0y1z0 = center + glm::vec3{-extents.x, extents.y, -extents.z};
    const glm::vec3 x1y1z0 = center + glm::vec3{extents.x, extents.y, -extents.z};
    const glm::vec3 x0y1z1 = center + glm::vec3{-extents.x, extents.y, extents.z};
    const glm::vec3 x1y1z1 = center + glm::vec3{extents.x, extents.y, extents.z};

    drawLine(x0y0z0, x1y0z0, color);
    drawLine(x0y0z0, x0y0z1, color);
    drawLine(x1y0z1, x1y0z0, color);
    drawLine(x1y0z1, x0y0z1, color);

    drawLine(x0y1z0, x1y1z0, color);
    drawLine(x0y1z0, x0y1z1, color);
    drawLine(x1y1z1, x1y1z0, color);
    drawLine(x1y1z1, x0y1z1, color);

    drawLine(x0y0z0, x0y1z0, color);
    drawLine(x1y0z0, x1y1z0, color);
    drawLine(x0y0z1, x0y1z1, color);
    drawLine(x1y0z1, x1y1z1, color);
}

void DebugDraw::drawCuboidMinMax(const glm::vec3 &min, const glm::vec3 &max, const glm::vec3 &color) {
    drawCuboid((max + min) / 2.0f, (max - min) * 0.5f, color);
}

void DebugDraw::drawSphere(const glm::vec3 &center, float radius, const glm::vec3& color) {
    constexpr int32_t resolution = 32;

    for (int32_t i = 0 ; i < resolution ; ++i) {
        const float theta0 = (float)i / (float)resolution * glm::tau<float>();
        const float theta1 = (float)(i+1) / (float)resolution * glm::tau<float>();

        drawLine(
            center + glm::vec3(cos(theta0), 0.0f, sin(theta0)) * radius,
            center + glm::vec3(cos(theta1), 0.0f, sin(theta1)) * radius);

        drawLine(
            center + glm::vec3(cos(theta0), sin(theta0), 0.0f) * radius,
            center + glm::vec3(cos(theta1), sin(theta1), 0.0f) * radius);

        drawLine(
            center + glm::vec3(0.0f, sin(theta0), cos(theta0)) * radius,
            center + glm::vec3(0.0f, sin(theta1), cos(theta1)) * radius);
    }
}

// void DebugDraw::drawFrustum(const glm::mat4 &view_projection, const glm::vec3& color)
// {
//     const std::vector<glm::vec3> points = extractFrustumCornersWorldSpace(view_projection);

//     drawLine(points[0], points[1], color);
//     drawLine(points[0], points[2], color);
//     drawLine(points[3], points[1], color);
//     drawLine(points[3], points[2], color);

//     drawLine(points[0], points[4], color);
//     drawLine(points[1], points[5], color);
//     drawLine(points[2], points[6], color);
//     drawLine(points[3], points[7], color);

//     drawLine(points[4], points[5], color);
//     drawLine(points[4], points[6], color);
//     drawLine(points[7], points[5], color);
//     drawLine(points[7], points[6], color);
// }

void DebugDraw::draw(VkCommandBuffer cb, const glm::mat4& view_projection) {
    if (m_vertices.size() < 2)
        return;

    const int32_t vertexCount = m_vertices.size();
    const int32_t verticesSizeBytes = m_vertices.size() * sizeof(DebugDrawVertex);

    m_shaderData.viewProjection = view_projection;
    m_uniformBuffer.upload(&m_shaderData, sizeof(DebugDrawShaderData));
    m_vertexBuffer.upload(m_vertices.data(), verticesSizeBytes);

    const auto& ctx = Context::instance();

    auto pass = RenderPass()
        .defaultViewportScissor()
        .color(
            ctx.getSwapchainImage(),
            ctx.getSwapchainImageView())
        .depth(
            ctx.getDepthTexture());

    pass.execute([&]() {
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.pipeline);

        m_uniformBuffer.pushConstant(m_pipeline.layout, VkShaderStageFlagBits(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));

        VkDeviceSize _vOffset{ 0 };
        vkCmdBindVertexBuffers(cb, 0, 1, &m_vertexBuffer.buffer, &_vOffset);
        // vkCmdSetLineWidth(cb, 1.0f);

        vkCmdDraw(cb, vertexCount, 1, 0, 0);
    });
}

void DebugDraw::flush() {
    m_vertices.clear();
}

void DebugDraw::drawAndFlush(VkCommandBuffer cb, const glm::mat4& view_projection) {
    draw(cb, view_projection);
    flush();
}
