class Context
    Queue
    Device
    window, surface
    Swapchain


loadRessource("shader.slang")


VkPipelineShaderStageCreateInfo

Texture (bindless)

Buffer
    write
    read

shaders
model
procedural geometry


renderpass.setup()
    .image(texture, 0)
    .image(texture, 1)
    .image(texture, 2)
    .buffer(buf, 0)


beginRendering()

renderPass.begin()
// Render stuff...
renderPass.end()

EndRendering()

// ---------------------

vkBeginCommandBuffer

vkCmdBeginRendering
vkCmdEndRendering

vkEndCommandBuffer
