#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "Context.hpp"

class Buffer {
public:
    Buffer() = default;

    void create(VkDeviceSize size, VkBufferUsageFlags usage);
    void upload(const void *data, size_t size);
    void destroy();
    VkDeviceSize size() const;
    VkBuffer& get();

private:
    VkBuffer m_buffer{ VK_NULL_HANDLE };
    VmaAllocation m_allocation{ VK_NULL_HANDLE };
    VmaAllocationInfo m_allocationInfo{};
};



// struct Vertex {
//     glm::vec3 pos;
//     glm::vec3 color;
//     glm::vec2 uv;
// };

// const VkDeviceSize indexCount{6};
// std::vector<Vertex> vertices{
//     {{-1,  1, 0}, {1, 0, 0}, {0, 0}},
//     {{ 1,  1, 0}, {0, 1, 0}, {1, 0}},
//     {{-1, -1, 0}, {0, 0, 1}, {0, 1}},
//     {{ 1, -1, 0}, {0, 1, 1}, {1, 1}},
// };
// std::vector<uint16_t> indices{
//     0, 2, 1,
//     2, 1, 3,
// };

// VkDeviceSize vBufSize{ sizeof(Vertex) * vertices.size() };
// VkDeviceSize iBufSize{ sizeof(uint16_t) * indices.size() };
// VkBufferCreateInfo bufferCI{
//     .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//     .size = vBufSize + iBufSize,
//     .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
// };


// VkBuffer vBuffer{ VK_NULL_HANDLE };
// VmaAllocation vBufferAllocation{ VK_NULL_HANDLE };
// VmaAllocationCreateInfo vBufferAllocCI{
//     .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
//     .usage = VMA_MEMORY_USAGE_AUTO
// };
// VmaAllocationInfo vBufferAllocInfo{};
// chk(vmaCreateBuffer(allocator, &bufferCI, &vBufferAllocCI, &vBuffer, &vBufferAllocation, &vBufferAllocInfo));

// memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
// memcpy(((char*)vBufferAllocInfo.pMappedData) + vBufSize, indices.data(), iBufSize);
