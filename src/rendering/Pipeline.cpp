#include <Context.hpp>
#include "Pipeline.hpp"

void Pipeline::destroy() {
    if (pipeline == VK_NULL_HANDLE)
        return;

    const auto device = Context::instance().getDevice();
    vkDeviceWaitIdle(device);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);
}
