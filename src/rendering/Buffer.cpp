#include "Buffer.hpp"
#include "Context.hpp"

void Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage) {
    const auto& ctx = Context::instance();

    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
    };

    VmaAllocationCreateInfo bufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    vmaCreateBuffer(
        ctx.getVmaAllocator(),
        &bufferCI, &bufferAllocCI,
        &buffer,
        &allocation,
        &allocationInfo);

    VkBufferDeviceAddressInfo uBufferBdaInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer
    };
    deviceAddress = vkGetBufferDeviceAddress(ctx.getDevice(), &uBufferBdaInfo);
}

void Buffer::upload(const void *data, size_t size) {
    memcpy(allocationInfo.pMappedData, data, size);
}

void Buffer::destroy() {
    vkDeviceWaitIdle(Context::instance().getDevice());
    vmaDestroyBuffer(
        Context::instance().getVmaAllocator(),
        buffer,
        allocation);

    buffer = VK_NULL_HANDLE;
    allocation = VK_NULL_HANDLE;
    allocationInfo = {};
}

VkDeviceSize Buffer::size() const {
    return allocationInfo.size;
}
