#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

struct Texture {
    void create(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags imageUsageFlags, VkImageAspectFlags viewAspectMask);
    void destroy();

    VkImage image = nullptr;
    VkImageView imageView = nullptr;
    VmaAllocation allocation = nullptr;
};
