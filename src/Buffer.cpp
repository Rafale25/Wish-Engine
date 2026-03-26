#include "Buffer.hpp"

void Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage) {
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage
    };

    VmaAllocationCreateInfo bufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo vBufferAllocInfo{};
    vmaCreateBuffer(
        Context::instance().getVmaAllocator(),
        &bufferCI, &bufferAllocCI,
        &m_buffer,
        &m_allocation,
        &m_allocationInfo);

    VkBufferDeviceAddressInfo uBufferBdaInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = m_buffer
    };
    m_deviceAddress = vkGetBufferDeviceAddress(Context::instance().getDevice(), &uBufferBdaInfo);
}

void Buffer::upload(const void *data, size_t size) {
    memcpy(m_allocationInfo.pMappedData, data, size);
}

void Buffer::destroy() {
    vmaDestroyBuffer(
        Context::instance().getVmaAllocator(),
        m_buffer,
        m_allocation);

    m_buffer = VK_NULL_HANDLE;
    m_allocation = VK_NULL_HANDLE;
    m_allocationInfo = {};
}

VkDeviceSize Buffer::size() const {
    return m_allocationInfo.size;
}

VkBuffer& Buffer::get() {
    return m_buffer;
}
