#pragma once

#include "Buffer.hpp"
#include "View.hpp"
#include "DescriptorSet.hpp"
#include "UniformBuffer.hpp"
#include "CameraFps.hpp"
#include "Pipeline.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.h>

class Buffer;

struct ShaderData {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::vec4 viewPosition{0.0f};
    glm::vec4 viewDirection{0.0f};
    float time{0.0f};
};

class App: public View {
public:
    App();
    ~App();

    void onUpdate(double time_since_start, float dt);
    void onDraw(double time_since_start, float dt);
    void onResize(int width, int height);

    void onKeyPress(int key);
    void onMouseMotion(int x, int y, int dx, int dy);

private:
    UniformBuffer m_uniformBuffer{};
    ShaderData m_shaderData{};
    Pipeline m_pipelineSkybox{};
    Pipeline m_pipeline{};
    Buffer m_bufferVertex{};
    Buffer m_bufferIndices{};

    Buffer m_cubemapBufferVertex{};

    Texture gigachad{};
    Texture m_cubemap{};
    DescriptorSet m_descriptorSet{};
    DescriptorSet m_descriptorSetCubemap{};

    FPSCamera m_camera{};
};
