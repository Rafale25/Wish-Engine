#pragma once

#include "Buffer.hpp"
#include "View.hpp"
#include <vulkan/vulkan.h>
#include "GraphicsPipelineBuilder.hpp"
#include "UniformBuffer.hpp"

class Buffer;

struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    float time{0.0f};
};

class App: public View {
public:
    App(Context& ctx);

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
