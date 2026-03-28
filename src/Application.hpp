#pragma once

#include "Buffer.hpp"
#include "View.hpp"
#include <vulkan/vulkan.h>
#include <glm/ext/matrix_float4x4.hpp>
#include "GraphicsPipelineBuilder.hpp"
#include "UniformBuffer.hpp"

class Buffer;

struct ShaderData {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    float time{0.0f};
};

class App: public View {
public:
    App();
    ~App();

    void onUpdate(double time_since_start, float dt);
    void onDraw(double time_since_start, float dt);
    void onResize(int width, int height);

private:
    UniformBuffer m_uniformBuffer{};
    ShaderData m_shaderData{};
    Pipeline m_pipeline{};
    Buffer m_bufferVertex{};
    Buffer m_bufferIndices{};
};
