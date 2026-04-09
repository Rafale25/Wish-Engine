#pragma once

#include "Buffer.hpp"
#include "UniformBuffer.hpp"
#include "Pipeline.hpp"
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>

constexpr glm::vec3 DEFAULT_COLOR = {1.0f, 0.0f, 0.0f};

struct DebugDrawVertex {
    glm::vec3 pos;
    int color;
};

struct DebugDrawShaderData {
    glm::mat4 viewProjection{1.0f};
};

class DebugDraw {
public:
    void drawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color = DEFAULT_COLOR);
    void drawRay(const glm::vec3& start, const glm::vec3& v, const glm::vec3& color = DEFAULT_COLOR);
    void drawCube(const glm::vec3 &center, float size = 1.0f, const glm::vec3 &color = DEFAULT_COLOR);
    void drawCuboid(const glm::vec3& center, const glm::vec3& extents = {0.5f, 0.5f, 0.5f}, const glm::vec3& color = DEFAULT_COLOR);
    void drawCuboidMinMax(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color = DEFAULT_COLOR);
    void drawSphere(const glm::vec3& center, float radius = 1.0f, const glm::vec3& color = DEFAULT_COLOR);
    void drawFrustum(const glm::mat4& view_projection, const glm::vec3& color = DEFAULT_COLOR);

    void draw(VkCommandBuffer cb, const glm::mat4& view_projection);
    void flush();
    void drawAndFlush(VkCommandBuffer cb, const glm::mat4& view_projection);

    static DebugDraw& instance() {
        static DebugDraw instance;
        return instance;
    }

private:
    DebugDraw();
    ~DebugDraw();

    DebugDraw(const DebugDraw&) = delete;
    DebugDraw& operator=(const DebugDraw&) = delete;
    DebugDraw(DebugDraw&&) = delete;
    DebugDraw& operator=(DebugDraw&&) = delete;

private:
    DebugDrawShaderData m_shaderData{};
    UniformBuffer m_uniformBuffer{};
    Buffer m_vertexBuffer{};
    Pipeline m_pipeline{};
    std::vector<DebugDrawVertex> m_vertices;
};
