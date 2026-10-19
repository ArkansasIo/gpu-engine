#pragma once

#include "gpu_engine/backend.hpp"

#include <cstdint>
#include <memory>

namespace gpu_engine::vulkan {

struct DeviceConfig {
    std::uint32_t preferred_device{0};
    bool prefer_discrete{true};
};

class Device {
public:
    Device();
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Status initialize(const DeviceConfig& config = {});
    void shutdown() noexcept;
    bool is_initialized() const noexcept;

    const backend::DeviceInfo& info() const noexcept;
    std::uint32_t graphics_queue_family() const noexcept;
    std::uint32_t compute_queue_family() const noexcept;

    /// Submit an empty command-buffer batch and wait for the queue to idle.
    Status synchronize() noexcept;

    /// Opaque native handles used by the Vulkan resource layer.
    void* native_device_handle() const noexcept;
    void* native_physical_device_handle() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gpu_engine::vulkan
