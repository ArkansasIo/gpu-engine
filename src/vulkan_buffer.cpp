#include "gpu_engine/vulkan_buffer.hpp"

#if defined(GPU_ENGINE_HAS_VULKAN)
#include <vulkan/vulkan.h>
#endif

#include <cstring>
#include <limits>
#include <vector>

namespace gpu_engine::vulkan {

struct Buffer::Impl {
    Device* owner{nullptr};
    memory::BufferDesc desc{};
#if defined(GPU_ENGINE_HAS_VULKAN)
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkPhysicalDevice physical{VK_NULL_HANDLE};
#endif
};

Buffer::Buffer() : impl_(std::make_unique<Impl>()) {}
Buffer::~Buffer() { destroy(); }

#if defined(GPU_ENGINE_HAS_VULKAN)
static VkBufferUsageFlags to_vk_usage(memory::BufferUsage usage) {
    VkBufferUsageFlags flags = 0;
    const auto bits = static_cast<std::uint32_t>(usage);
    if (bits & static_cast<std::uint32_t>(memory::BufferUsage::Vertex)) flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (bits & static_cast<std::uint32_t>(memory::BufferUsage::Index)) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (bits & static_cast<std::uint32_t>(memory::BufferUsage::Uniform)) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (bits & static_cast<std::uint32_t>(memory::BufferUsage::Storage)) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (bits & static_cast<std::uint32_t>(memory::BufferUsage::TransferSource)) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (bits & static_cast<std::uint32_t>(memory::BufferUsage::TransferDestination)) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return flags;
}

static std::uint32_t find_memory_type(VkPhysicalDevice physical, std::uint32_t type_bits,
                                      VkMemoryPropertyFlags required) {
    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(physical, &properties);
    for (std::uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
        if ((type_bits & (1u << i)) &&
            (properties.memoryTypes[i].propertyFlags & required) == required) return i;
    }
    return std::numeric_limits<std::uint32_t>::max();
}
#endif

Status Buffer::create(Device& device, const memory::BufferDesc& desc) {
    destroy();
    if (!device.is_initialized()) return Status::failure("Vulkan device is not initialized");
    if (desc.size_bytes == 0) return Status::failure("Vulkan buffer size must be greater than zero");
#if defined(GPU_ENGINE_HAS_VULKAN)
    // Device is intentionally opaque; the first implementation uses an internal access bridge.
    // The concrete handles are exposed by the implementation unit through this local adapter.
    struct DeviceAccess { Device::Impl* impl; };
    auto* access = reinterpret_cast<DeviceAccess*>(&device);
    (void)access;
    return Status::failure("Vulkan buffer allocation requires the device handle bridge");
#else
    (void)desc;
    return Status::failure("Vulkan backend was not compiled: Vulkan SDK not found");
#endif
}

void Buffer::destroy() noexcept {
#if defined(GPU_ENGINE_HAS_VULKAN)
    if (impl_->device != VK_NULL_HANDLE) {
        if (impl_->buffer != VK_NULL_HANDLE) vkDestroyBuffer(impl_->device, impl_->buffer, nullptr);
        if (impl_->memory != VK_NULL_HANDLE) vkFreeMemory(impl_->device, impl_->memory, nullptr);
    }
    impl_->buffer = VK_NULL_HANDLE;
    impl_->memory = VK_NULL_HANDLE;
    impl_->device = VK_NULL_HANDLE;
    impl_->physical = VK_NULL_HANDLE;
#endif
    impl_->owner = nullptr;
    impl_->desc = {};
}

bool Buffer::is_initialized() const noexcept {
#if defined(GPU_ENGINE_HAS_VULKAN)
    return impl_ && impl_->buffer != VK_NULL_HANDLE;
#else
    return false;
#endif
}

std::size_t Buffer::size_bytes() const noexcept { return impl_->desc.size_bytes; }

Status Buffer::upload(const void* data, std::size_t size, std::size_t offset) {
    if (!is_initialized()) return Status::failure("Vulkan buffer is not initialized");
    if (!data || offset > size_bytes() || size > size_bytes() - offset)
        return Status::failure("Vulkan buffer upload range is invalid");
#if defined(GPU_ENGINE_HAS_VULKAN)
    void* mapped = nullptr;
    if (vkMapMemory(impl_->device, impl_->memory, offset, size, 0, &mapped) != VK_SUCCESS)
        return Status::failure("Failed to map Vulkan buffer memory");
    std::memcpy(mapped, data, size);
    vkUnmapMemory(impl_->device, impl_->memory);
    return Status::success();
#else
    return Status::failure("Vulkan backend was not compiled");
#endif
}

Status Buffer::read(void* data, std::size_t size, std::size_t offset) {
    if (!is_initialized()) return Status::failure("Vulkan buffer is not initialized");
    if (!data || offset > size_bytes() || size > size_bytes() - offset)
        return Status::failure("Vulkan buffer read range is invalid");
#if defined(GPU_ENGINE_HAS_VULKAN)
    void* mapped = nullptr;
    if (vkMapMemory(impl_->device, impl_->memory, offset, size, 0, &mapped) != VK_SUCCESS)
        return Status::failure("Failed to map Vulkan buffer memory");
    std::memcpy(data, mapped, size);
    vkUnmapMemory(impl_->device, impl_->memory);
    return Status::success();
#else
    return Status::failure("Vulkan backend was not compiled");
#endif
}

} // namespace gpu_engine::vulkan
