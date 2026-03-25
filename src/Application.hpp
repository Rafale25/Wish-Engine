#pragma once

#include "Buffer.hpp"
#include "View.hpp"
#include <vulkan/vulkan.h>
#include "GraphicsPipelineBuilder.hpp"

class Buffer;

class App: public View {
public:
    App(Context& ctx);

    void onUpdate(double time_since_start, float dt);
    void onDraw(double time_since_start, float dt);
    void onResize(int width, int height);

private:
    Pipeline m_pipeline{};
    Buffer m_bufferVertex{};
    Buffer m_bufferIndices{};
};
