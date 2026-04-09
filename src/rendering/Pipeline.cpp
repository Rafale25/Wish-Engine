#include <Context.hpp>
#include "Pipeline.hpp"

void Pipeline::destroy() {
    const auto device = Context::instance().getDevice();
    vkDeviceWaitIdle(device);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);
}
