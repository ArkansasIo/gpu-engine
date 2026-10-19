#include "gpu_engine/vulkan_backend.hpp"

#if defined(GPU_ENGINE_HAS_VULKAN)
#include <vulkan/vulkan.h>
#endif

#include <algorithm>
#include <utility>

namespace gpu_engine::vulkan {

struct Backend::Impl {
    bool initialized{false};
    std::vector<backend::DeviceInfo> devices;
#if defined(GPU_ENGINE_HAS_VULKAN)
    VkInstance instance{VK_NULL_HANDLE};
#endif
};

Backend::Backend() : impl_(std::make_unique<Impl>()) {}
Backend::~Backend() { shutdown(); }

gpu_engine::Backend Backend::type() const noexcept {
    return gpu_engine::Backend::Vulkan;
}

Status Backend::initialize() {
    if (impl_->initialized) return Status::success();
#if defined(GPU_ENGINE_HAS_VULKAN)
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "GPU Engine";
    app.applicationVersion = VK_MAKE_VERSION(0, 3, 0);
    app.pEngineName = "GPU Engine";
    app.engineVersion = VK_MAKE_VERSION(0, 3, 0);
    app.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create.pApplicationInfo = &app;
    if (vkCreateInstance(&create, nullptr, &impl_->instance) != VK_SUCCESS) {
        return Status::failure("Failed to create Vulkan instance");
    }

    std::uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(impl_->instance, &count, nullptr) != VK_SUCCESS) {
        shutdown();
        return Status::failure("Failed to enumerate Vulkan physical devices");
    }
    std::vector<VkPhysicalDevice> physical(count);
    if (count > 0 && vkEnumeratePhysicalDevices(impl_->instance, &count, physical.data()) != VK_SUCCESS) {
        shutdown();
        return Status::failure("Failed to read Vulkan physical devices");
    }

    for (VkPhysicalDevice physical_device : physical) {
        VkPhysicalDeviceProperties props{};
        VkPhysicalDeviceMemoryProperties memory{};
        vkGetPhysicalDeviceProperties(physical_device, &props);
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory);

        std::uint64_t local_memory = 0;
        for (std::uint32_t i = 0; i < memory.memoryHeapCount; ++i) {
            if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                local_memory = std::max(local_memory,
                    static_cast<std::uint64_t>(memory.memoryHeaps[i].size));
            }
        }

        std::uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queues.data());

        backend::DeviceInfo info;
        info.capabilities.name = props.deviceName;
        info.capabilities.backend = gpu_engine::Backend::Vulkan;
        info.capabilities.vendor_id = props.vendorID;
        info.capabilities.device_id = props.deviceID;
        info.capabilities.dedicated_memory_bytes = local_memory;
        for (std::uint32_t i = 0; i < queue_count; ++i) {
            backend::QueueInfo queue;
            queue.family_index = i;
            queue.graphics = (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
            queue.compute = (queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
            queue.present = false;
            info.queues.push_back(queue);
            info.capabilities.graphics_supported |= queue.graphics;
            info.capabilities.compute_supported |= queue.compute;
        }
        impl_->devices.push_back(std::move(info));
    }

    impl_->initialized = true;
    return Status::success();
#else
    return Status::failure("Vulkan backend was not compiled: Vulkan SDK not found");
#endif
}

void Backend::shutdown() noexcept {
    if (!impl_) return;
    impl_->devices.clear();
#if defined(GPU_ENGINE_HAS_VULKAN)
    if (impl_->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(impl_->instance, nullptr);
        impl_->instance = VK_NULL_HANDLE;
    }
#endif
    impl_->initialized = false;
}

bool Backend::is_initialized() const noexcept {
    return impl_ && impl_->initialized;
}

std::vector<backend::DeviceInfo> Backend::devices() const {
    return impl_ ? impl_->devices : std::vector<backend::DeviceInfo>{};
}

} // namespace gpu_engine::vulkan
