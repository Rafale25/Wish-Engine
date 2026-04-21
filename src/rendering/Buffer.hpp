#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class Buffer {
public:
    Buffer() = default;
    // ~Buffer() = default;
    ~Buffer() { destroy(); };

    void create(VkDeviceSize size, VkBufferUsageFlags usage);
    void upload(const void *data, size_t size, size_t offset=0);
    VkDeviceSize size() const;

private:
    void destroy();

    public:
    VkBuffer buffer{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VmaAllocationInfo allocationInfo{};
    VkDeviceAddress deviceAddress{};
};
