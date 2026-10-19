#include "gpu_engine/vulkan_device.hpp"

#if defined(GPU_ENGINE_HAS_VULKAN)
#include <vulkan/vulkan.h>
#endif

#include <limits>
#include <vector>

namespace gpu_engine::vulkan {

struct Device::Impl {
    bool initialized{false};
    backend::DeviceInfo info{};
    std::uint32_t graphics_family{std::numeric_limits<std::uint32_t>::max()};
    std::uint32_t compute_family{std::numeric_limits<std::uint32_t>::max()};
#if defined(GPU_ENGINE_HAS_VULKAN)
    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physical{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphics_queue{VK_NULL_HANDLE};
    VkQueue compute_queue{VK_NULL_HANDLE};
#endif
};

Device::Device() : impl_(std::make_unique<Impl>()) {}
Device::~Device() { shutdown(); }

Status Device::initialize(const DeviceConfig& config) {
    if (impl_->initialized) return Status::success();
#if defined(GPU_ENGINE_HAS_VULKAN)
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "GPU Engine";
    app.applicationVersion = VK_MAKE_VERSION(0, 3, 0);
    app.pEngineName = "GPU Engine";
    app.engineVersion = VK_MAKE_VERSION(0, 3, 0);
    app.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo instance_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instance_info.pApplicationInfo = &app;
    if (vkCreateInstance(&instance_info, nullptr, &impl_->instance) != VK_SUCCESS)
        return Status::failure("Vulkan instance creation failed");

    std::uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(impl_->instance, &count, nullptr) != VK_SUCCESS || count == 0) {
        shutdown();
        return Status::failure("No Vulkan physical devices are available");
    }
    std::vector<VkPhysicalDevice> physical(count);
    if (vkEnumeratePhysicalDevices(impl_->instance, &count, physical.data()) != VK_SUCCESS) {
        shutdown();
        return Status::failure("Failed to enumerate Vulkan physical devices");
    }

    std::uint32_t selected = config.preferred_device < count ? config.preferred_device : 0;
    if (config.prefer_discrete) {
        for (std::uint32_t i = 0; i < count; ++i) {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(physical[i], &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { selected = i; break; }
        }
    }
    impl_->physical = physical[selected];

    VkPhysicalDeviceProperties props{};
    VkPhysicalDeviceMemoryProperties memory{};
    vkGetPhysicalDeviceProperties(impl_->physical, &props);
    vkGetPhysicalDeviceMemoryProperties(impl_->physical, &memory);
    std::uint64_t local_memory = 0;
    for (std::uint32_t i = 0; i < memory.memoryHeapCount; ++i)
        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) local_memory += memory.memoryHeaps[i].size;

    std::uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(impl_->physical, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queues(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(impl_->physical, &queue_count, queues.data());
    for (std::uint32_t i = 0; i < queue_count; ++i) {
        const auto flags = queues[i].queueFlags;
        if (impl_->graphics_family == std::numeric_limits<std::uint32_t>::max() && (flags & VK_QUEUE_GRAPHICS_BIT)) impl_->graphics_family = i;
        if (impl_->compute_family == std::numeric_limits<std::uint32_t>::max() && (flags & VK_QUEUE_COMPUTE_BIT)) impl_->compute_family = i;
    }
    if (impl_->compute_family == std::numeric_limits<std::uint32_t>::max()) { shutdown(); return Status::failure("Selected Vulkan device has no compute queue"); }
    if (impl_->graphics_family == std::numeric_limits<std::uint32_t>::max()) impl_->graphics_family = impl_->compute_family;

    const float priority = 1.0f;
    std::vector<std::uint32_t> unique_families{impl_->compute_family};
    if (impl_->graphics_family != impl_->compute_family) unique_families.push_back(impl_->graphics_family);
    std::vector<VkDeviceQueueCreateInfo> queue_infos;
    for (auto family : unique_families) {
        VkDeviceQueueCreateInfo qi{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        qi.queueFamilyIndex = family; qi.queueCount = 1; qi.pQueuePriorities = &priority;
        queue_infos.push_back(qi);
    }
    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_info.queueCreateInfoCount = static_cast<std::uint32_t>(queue_infos.size());
    device_info.pQueueCreateInfos = queue_infos.data();
    device_info.pEnabledFeatures = &features;
    if (vkCreateDevice(impl_->physical, &device_info, nullptr, &impl_->device) != VK_SUCCESS) {
        shutdown();
        return Status::failure("Vulkan logical device creation failed");
    }
    vkGetDeviceQueue(impl_->device, impl_->graphics_family, 0, &impl_->graphics_queue);
    vkGetDeviceQueue(impl_->device, impl_->compute_family, 0, &impl_->compute_queue);
    impl_->info.capabilities.name = props.deviceName;
    impl_->info.capabilities.backend = gpu_engine::Backend::Vulkan;
    impl_->info.capabilities.vendor_id = props.vendorID;
    impl_->info.capabilities.device_id = props.deviceID;
    impl_->info.capabilities.dedicated_memory_bytes = local_memory;
    impl_->info.capabilities.graphics_supported = true;
    impl_->info.capabilities.compute_supported = true;
    impl_->initialized = true;
    return Status::success();
#else
    (void)config;
    return Status::failure("Vulkan backend was not compiled: Vulkan SDK not found");
#endif
}

void Device::shutdown() noexcept {
    if (!impl_) return;
#if defined(GPU_ENGINE_HAS_VULKAN)
    if (impl_->device != VK_NULL_HANDLE) { vkDeviceWaitIdle(impl_->device); vkDestroyDevice(impl_->device, nullptr); impl_->device = VK_NULL_HANDLE; }
    if (impl_->instance != VK_NULL_HANDLE) { vkDestroyInstance(impl_->instance, nullptr); impl_->instance = VK_NULL_HANDLE; }
    impl_->physical = VK_NULL_HANDLE;
#endif
    impl_->graphics_family = std::numeric_limits<std::uint32_t>::max();
    impl_->compute_family = std::numeric_limits<std::uint32_t>::max();
    impl_->info = {};
    impl_->initialized = false;
}

bool Device::is_initialized() const noexcept { return impl_ && impl_->initialized; }
const backend::DeviceInfo& Device::info() const noexcept { return impl_->info; }
std::uint32_t Device::graphics_queue_family() const noexcept { return impl_->graphics_family; }
std::uint32_t Device::compute_queue_family() const noexcept { return impl_->compute_family; }

void* Device::native_device_handle() const noexcept {
#if defined(GPU_ENGINE_HAS_VULKAN)
    return impl_ ? reinterpret_cast<void*>(impl_->device) : nullptr;
#else
    return nullptr;
#endif
}
void* Device::native_physical_device_handle() const noexcept {
#if defined(GPU_ENGINE_HAS_VULKAN)
    return impl_ ? reinterpret_cast<void*>(impl_->physical) : nullptr;
#else
    return nullptr;
#endif
}

Status Device::synchronize() noexcept {
#if defined(GPU_ENGINE_HAS_VULKAN)
    if (!is_initialized()) return Status::failure("Vulkan device is not initialized");
    return vkDeviceWaitIdle(impl_->device) == VK_SUCCESS ? Status::success() : Status::failure("Vulkan device synchronization failed");
#else
    return Status::failure("Vulkan backend was not compiled: Vulkan SDK not found");
#endif
}

} // namespace gpu_engine::vulkan
